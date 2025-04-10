#pragma once
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <iostream>
#include <stdexcept>
#include "proto/hello.grpc.pb.h"

template <typename T>
class MPointer {
private:
    static std::unique_ptr<memory_manager::MemoryManager::Stub> stub_;
    int id_;
    int next_id_;
    
    // Handle reference counting properly
    void increaseRefCount() {
        if (id_ != -1) {
            memory_manager::RefCountRequest request;
            request.set_id(id_);
            memory_manager::RefCountResponse response;
            grpc::ClientContext context;
            stub_->IncreaseRefCount(&context, request, &response);
        }
    }
    
    void decreaseRefCount() {
        if (id_ != -1) {
            memory_manager::RefCountRequest request;
            request.set_id(id_);
            memory_manager::RefCountResponse response;
            grpc::ClientContext context;
            stub_->DecreaseRefCount(&context, request, &response);
        }
    }

    // Proxy class to handle the dereference and assignment operations
    class ValueProxy {
    private:
        MPointer<T>& pointer;
    public:
        ValueProxy(MPointer<T>& ptr) : pointer(ptr) {}
        
        // Conversion operator to allow reading the value
        operator T() const {
            if (pointer.id_ == -1) {
                throw std::runtime_error("Invalid memory block ID");
            }
            
            memory_manager::GetRequest request;
            request.set_id(pointer.id_);
            
            memory_manager::GetResponse response;
            grpc::ClientContext context;
            grpc::Status status = pointer.stub_->Get(&context, request, &response);
            
            if (!status.ok()) {
                throw std::runtime_error("Failed to get value: " + status.error_message());
            }
            
            std::string valueStr = response.value();
            if (valueStr.find("No value assigned") != std::string::npos) {
                return T(); // Default value
            }
            
            if constexpr (std::is_same<T, int>::value) {
                return std::stoi(valueStr);
            } else if constexpr (std::is_same<T, float>::value) {
                return std::stof(valueStr);
            } else if constexpr (std::is_same<T, double>::value) {
                return std::stod(valueStr);
            } else {
                throw std::runtime_error("Unsupported type for conversion");
            }
        }
        
        // Assignment operator to allow writing the value
        ValueProxy& operator=(const T& new_value) {
            if (pointer.id_ == -1) {
                throw std::runtime_error("Invalid memory block ID");
            }
            
            memory_manager::SetRequest request;
            request.set_id(pointer.id_);
            
            if constexpr (std::is_same<T, int>::value || 
                        std::is_same<T, float>::value || 
                        std::is_same<T, double>::value) {
                request.set_value(std::to_string(new_value));
            } else {
                throw std::runtime_error("Unsupported type for conversion");
            }
            
            memory_manager::SetResponse response;
            grpc::ClientContext context;
            grpc::Status status = pointer.stub_->Set(&context, request, &response);
            
            if (!status.ok()) {
                throw std::runtime_error("Failed to set value: " + status.error_message());
            }
            
            return *this;
        }
    };

public:
    // Default constructor
    MPointer() : id_(-1), next_id_(-1) {}
    
    // Constructor with ID
    explicit MPointer(int id) : id_(id), next_id_(-1) {
        increaseRefCount();
    }
    
    // Copy constructor
    MPointer(const MPointer& other) : id_(other.id_), next_id_(other.next_id_) {
        increaseRefCount();
    }
    
    // Move constructor
    MPointer(MPointer&& other) noexcept : id_(other.id_), next_id_(other.next_id_) {
        other.id_ = -1;
        other.next_id_ = -1;
    }
    
    // Copy assignment operator
    MPointer& operator=(const MPointer& other) {
        if (this != &other) {  // This is comparing pointers, which is correct
            decreaseRefCount();
            id_ = other.id_;
            next_id_ = other.next_id_;
            increaseRefCount();
        }
        return *this;
    }
    
    // Move assignment operator - FIXED
    MPointer& operator=(MPointer&& other) noexcept {
        // Don't use the & operator here, compare the pointers themselves
        if (this != std::addressof(other)) {
            decreaseRefCount();
            id_ = other.id_;
            next_id_ = other.next_id_;
            other.id_ = -1;
            other.next_id_ = -1;
        }
        return *this;
    }
    
    // Initialize stub
    static void Init(const std::string& server_address) {
        stub_ = memory_manager::MemoryManager::NewStub(
            grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())
        );
    }
    
    // Create a new memory block
    static MPointer<T> New() {
        if (!stub_) {
            throw std::runtime_error("MPointer not initialized. Call Init() first.");
        }
        
        memory_manager::CreateRequest request;
        request.set_size(sizeof(T));
        
        // Set the appropriate type based on T
        if (std::is_same<T, int>::value) {
            request.set_type("int");
        } else if (std::is_same<T, float>::value) {
            request.set_type("float");
        } else if (std::is_same<T, double>::value) {
            request.set_type("double");
        } else {
            request.set_type("generic");
        }
        
        memory_manager::CreateResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->Create(&context, request, &response);
        
        if (!status.ok()) {
            throw std::runtime_error("Failed to create memory block: " + status.error_message());
        }
        
        return MPointer<T>(response.id());
    }
    
    // The dereference operator now returns a proxy object
    ValueProxy operator*() {
        return ValueProxy(*this);
    }
    
    // Get ID (address) - NOT using the & operator anymore
    int getId() const {
        return id_;
    }
    
    // Set the next pointer for linked list
    void setNext(const MPointer<T>& next) {
        next_id_ = next.id_;  // Use the id_ directly
    }
    
    // Get the next pointer
    MPointer<T> getNext() const {
        if (next_id_ == -1) {
            return MPointer<T>();
        }
        return MPointer<T>(next_id_);
    }
    
    // Check if this is a null pointer
    bool isNull() const {
        return id_ == -1;
    }
    
    // Destructor
    ~MPointer() {
        decreaseRefCount();
    }
};

// Static member initialization
template <typename T>
std::unique_ptr<memory_manager::MemoryManager::Stub> MPointer<T>::stub_ = nullptr;