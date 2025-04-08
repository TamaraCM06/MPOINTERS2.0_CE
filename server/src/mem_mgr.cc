#include <grpcpp/grpcpp.h>
#include "proto/hello.grpc.pb.h"
#include "proto/hello.pb.h"
#include <iostream>
#include <string>
#include <getopt.h>
#include <cstdlib>
#include "dumps/dumps.h"
#include "services/create/create_service.h"
#include "services/set/set_service.h"
#include "services/get/get_service.h"
#include "services/increase_ref/increase_ref_service.h"
#include "services/decrease_ref/decrease_ref_service.h"

class MemoryManager {
    private:
        void* memory_chunk;
        size_t memory_chunk_size;
        Dumps dumps;
    

public:
    MemoryManager(size_t size_mb, const std::string& folder)
    : memory_chunk_size(size_mb * 1024 * 1024),
    dumps(folder, memory_chunk_size) {
    // Allocate memory block
    memory_chunk = malloc(memory_chunk_size);
    if (!memory_chunk) {
        std::cerr << "Failed to allocate " << size_mb << "MB of memory" << std::endl;
        exit(1);
    }

    std::cout << "Allocated " << size_mb << "MB of memory" << std::endl;
    }

        ~MemoryManager() {
        if (memory_chunk) {
            free(memory_chunk);
        }
    }

    int create(int size, const std::string& type) {
        return ::create(size, type);
    }

    bool set(int id, const std::string& value) {
        return ::set(id, value);
    }

    std::string get(int id) {
        return ::get(id);
    }

    int increaseRefCount(int id) {
        return ::increaseRefCount(id);
    }

    int decreaseRefCount(int id) {
        return ::decreaseRefCount(id);
    }
};

class MemoryManagerServiceImpl final : public memory_manager::MemoryManager::Service {
    private:
        MemoryManager* memory_manager;
    
    public:
        explicit MemoryManagerServiceImpl(MemoryManager* mem_mgr) : memory_manager(mem_mgr) {}
    
        grpc::Status Create(::grpc::ServerContext* context, const memory_manager::CreateRequest* request,
                            memory_manager::CreateResponse* response) override {
            int id = memory_manager->create(request->size(), request->type());
            response->set_id(id);
            response->set_success(true);
            response->set_message("Create operation successful. Created ID: " + std::to_string(id));
            return grpc::Status::OK;
        }
    
        grpc::Status Set(::grpc::ServerContext* context, const memory_manager::SetRequest* request,
                         memory_manager::SetResponse* response) override {
            bool success = memory_manager->set(request->id(), request->value());
            response->set_success(success);
            response->set_message(success ? "Set operation successful for ID: " + std::to_string(request->id())
                                           : "Set operation failed for ID: " + std::to_string(request->id()));
            return grpc::Status::OK;
        }
    
        grpc::Status Get(::grpc::ServerContext* context, const memory_manager::GetRequest* request,
                         memory_manager::GetResponse* response) override {
            std::string value = memory_manager->get(request->id());
            response->set_value(value);
            response->set_success(true);
            response->set_message("Get operation successful. Value: " + value);
            return grpc::Status::OK;
        }
    
        grpc::Status IncreaseRefCount(::grpc::ServerContext* context, const memory_manager::RefCountRequest* request,
                                      memory_manager::RefCountResponse* response) override {
            int new_ref_count = memory_manager->increaseRefCount(request->id());
            response->set_new_ref_count(new_ref_count);
            response->set_success(true);
            response->set_message("IncreaseRefCount operation successful. New RefCount: " + std::to_string(new_ref_count));
            return grpc::Status::OK;
        }
    
        grpc::Status DecreaseRefCount(::grpc::ServerContext* context, const memory_manager::RefCountRequest* request,
                                      memory_manager::RefCountResponse* response) override {
            int new_ref_count = memory_manager->decreaseRefCount(request->id());
            response->set_new_ref_count(new_ref_count);
            response->set_success(true);
            response->set_message("DecreaseRefCount operation successful. New RefCount: " + std::to_string(new_ref_count));
            return grpc::Status::OK;
        }
    };
    
    void parse_arguments(int argc, char* argv[], int& port, size_t& mem_size, std::string& dump_folder) {
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"memsize", required_argument, 0, 'm'},
            {"dumpFolder", required_argument, 0, 'd'},
            {0, 0, 0, 0}
        };
    
        int opt, option_index = 0;
        while ((opt = getopt_long(argc, argv, "p:m:d:", long_options, &option_index)) != -1) {
            switch (opt) {
                case 'p':
                    port = std::atoi(optarg);
                    break;
                case 'm':
                    mem_size = std::atoi(optarg);
                    break;
                case 'd':
                    dump_folder = optarg;
                    break;
                default:
                    throw std::invalid_argument("Invalid command-line arguments");
            }
        }
    }
    
    int main(int argc, char* argv[]) {
        try {
            int port = 9999;
            size_t mem_size = 64;
            std::string dump_folder = "./dumps";
    
            parse_arguments(argc, argv, port, mem_size, dump_folder);
    
            MemoryManager memory_manager(mem_size, dump_folder);
    
            std::string server_address = "0.0.0.0:" + std::to_string(port);
            MemoryManagerServiceImpl service(&memory_manager);
    
            grpc::ServerBuilder builder;
            builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);
            std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
            std::cout << "Server listening on " << server_address << std::endl;
            server->Wait();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    
        return EXIT_SUCCESS;
    }