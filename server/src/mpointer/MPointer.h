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
    static std::unique_ptr<memory_manager::MemoryManager::Stub> stub_; // GRPC stub for communication
    int id_; // ID of the memory block managed by the server

    // Private constructor to initialize with an ID
    MPointer(int id) : id_(id) {}

public:
    // Static method to initialize the GRPC stub with the server address
    static void Init(const std::string& server_address) {
        stub_ = memory_manager::MemoryManager::NewStub(
            grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
    }

    // Static method to create a new MPointer
    static MPointer<T> New() {
        memory_manager::CreateRequest request;
        request.set_size(sizeof(T));
        request.set_type(typeid(T).name());

        memory_manager::CreateResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Create(&context, request, &response);
        if (!status.ok()) {
            throw std::runtime_error("Failed to create memory block: " + status.error_message());
        }

        return MPointer<T>(response.id());
    }

    // Overloaded dereference operator (*) to get or set the value
    T& operator*() {
        static T value; // Temporary storage for the value
        memory_manager::GetRequest request;
        request.set_id(id_);

        memory_manager::GetResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Get(&context, request, &response);
        if (!status.ok()) {
            throw std::runtime_error("Failed to get value: " + status.error_message());
        }

        memcpy(&value, response.value().data(), sizeof(T));
        return value;
    }

    // Overloaded assignment operator (=) to copy the ID and increase reference count
    MPointer<T>& operator=(const MPointer<T>& other) {
        if (id_ != other.id_) { // Compare IDs instead of pointers
            memory_manager::RefCountRequest request;
            request.set_id(other.id_);

            memory_manager::RefCountResponse response;
            grpc::ClientContext context;

            grpc::Status status = stub_->IncreaseRefCount(&context, request, &response);
            if (!status.ok()) {
                throw std::runtime_error("Failed to increase reference count: " + status.error_message());
            }

            id_ = other.id_;
        }
        return *this;
    }

    // Overloaded address-of operator (&) to return the ID
    int operator&() const {
        return id_;
    }

    // Destructor to notify the server to decrease the reference count
    ~MPointer() {
        memory_manager::RefCountRequest request;
        request.set_id(id_);

        memory_manager::RefCountResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->DecreaseRefCount(&context, request, &response);
        if (!status.ok()) {
            std::cerr << "Failed to decrease reference count: " << status.error_message() << std::endl;
        }
    }
};

// Static member initialization
template <typename T>
std::unique_ptr<memory_manager::MemoryManager::Stub> MPointer<T>::stub_ = nullptr;