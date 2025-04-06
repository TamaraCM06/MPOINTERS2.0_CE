#include <grpcpp/grpcpp.h>
#include "proto/hello.grpc.pb.h"
#include "proto/hello.pb.h"
#include <iostream>
#include <string>
#include <thread>
#include <getopt.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>

class MemoryManager {
    private:

    void* memory_chunk;
    size_t memory_chunk_size;
    std::string dump_folder;

    void memory_dump() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
            
        std::stringstream filename;
        filename << dump_folder << "/memory_dump_";
        
        std::tm tm_buf;
        localtime_r(&now_time_t, &tm_buf);
        
        filename << std::put_time(&tm_buf, "%Y%m%d_%H%M%S_") << std::setfill('0') 
                << std::setw(3) << now_ms.count() << ".txt";
                
        std::ofstream dump_file(filename.str());
        if (dump_file.is_open()) {
            dump_file << "Memory dump at " << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") 
                    << "." << now_ms.count() << std::endl;
            dump_file << "Memory size: " << memory_chunk_size << " bytes" << std::endl;

            dump_file.close();
        }
    }


public:
    MemoryManager(size_t size_mb, const std::string& folder) : 
        memory_chunk_size(size_mb * 1024 * 1024), 
        dump_folder(folder) {
        
        // Create directory if it doesn't exist
        std::filesystem::create_directories(dump_folder);
        
        // Allocate memory block
        memory_chunk = malloc(memory_chunk_size);
        if (!memory_chunk) {
            std::cerr << "Failed to allocate " << size_mb << "MB of memory" << std::endl;
            exit(1);
        }
        
        std::cout << "Allocated " << size_mb << "MB of memory" << std::endl;
        memory_dump();
    }
    
    ~MemoryManager() {
        if (memory_chunk) {
            free(memory_chunk);
        }
    }
    
    // Memory Manager unctions here (Create, Set, Get, IncreaseRefCount, DecreaseRefCount)
};



class ProcessingImpl final : public ProcessingServices::Service {
    private:
        MemoryManager* memory_manager;
    public:
        ProcessingImpl(MemoryManager* mem_mgr) : memory_manager(mem_mgr) {}
        ~ProcessingImpl() noexcept override = default;
        grpc::Status computeSum(
            ::grpc::ServerContext* context, const Point2* request, Numeric* response) override{
            std::cout << "Called!!!" << std::endl;
            response->set_value(request->x() + request->y() + request->z());

            return grpc::Status::OK;
            }
};

int main(int argc, char* argv[]) {

    // Default values
    int port = 9999;
    size_t mem_size = 64; 
    std::string dump_folder = "./dumps";
    
    // Parse command line arguments
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"memsize", required_argument, 0, 'm'},
        {"dumpFolder", required_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    
    // Setting command line arguments to variables / 3 required if initializing mem_mgr

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
                return 1;
        }
    }
    //Initialize Memory Manager
    MemoryManager memory_manager(mem_size, dump_folder);

    // Initialize server on port
    std::string server_address = "0.0.0.0:" + std::to_string(port);
    
    ProcessingImpl service(&memory_manager);

    // Create gRPC server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> mem_mgr(builder.BuildAndStart());
    
    // Print results
    std::cout << "Server listening on " << server_address << std::endl;
    std::cout << "Memory Manager initialized with " << mem_size << "MB" << std::endl;
    std::cout << "Memory dumps will be stored in " << dump_folder << std::endl;
    
    mem_mgr->Wait();
    return 0;
}