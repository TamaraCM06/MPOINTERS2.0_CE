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
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <atomic>

//Base for piece of memory randomly grabbed from Memory Chunk of Memory Manager
struct MemoryBlock{
    void* address;
    size_t size;
    std::string type;
    int ref_count;
    bool is_free;
};

class MemoryManager {
    private:

        void* memory_chunk;
        size_t memory_chunk_size;
        std::string dump_folder;

        // Track allocated blocks
        std::unordered_map<int, MemoryBlock> allocated_blocks;
        int next_block_id = 1;

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

                dump_file << "Allocated blocks:" << std::endl;
                for (const auto& pair : allocated_blocks) {
                    const auto& id = pair.first;
                    const auto& block = pair.second;
                    dump_file << "  Block ID: " << id 
                             << ", Size: " << block.size 
                             << ", Type: " << block.type 
                             << ", Ref Count: " << block.ref_count
                             << ", Address Offset: " << (static_cast<char*>(block.address) - static_cast<char*>(memory_chunk))
                             << std::endl;
                }
                dump_file.close();
            }
        }

        //Find memory block for allocation
    void* find_free_memory(size_t size) {

        //First memory allocation petition
        char* mem_ptr = static_cast<char*>(memory_chunk);
        char* end_ptr = mem_ptr + memory_chunk_size;
        
        //Keep track of used memory regions
        std::vector<std::pair<char*, char*>> used_regions;
        for (const auto& pair : allocated_blocks) {
            if (!pair.second.is_free) {
                char* start = static_cast<char*>(pair.second.address);
                char* end = start + pair.second.size;
                used_regions.push_back({start, end});
            }
        }
        
        //Sort by start address
        std::sort(used_regions.begin(), used_regions.end());
        
        //Check if there's space before the first block
        if (used_regions.empty() || used_regions[0].first - mem_ptr >= size) {
            return mem_ptr;
        }
        
        //Check for gaps between used regions
        for (size_t i = 0; i < used_regions.size() - 1; i++) {
            char* current_end = used_regions[i].second;
            char* next_start = used_regions[i + 1].first;
            
            if (next_start - current_end >= size) {
                return current_end;
            }
        }
        
        //Check if there's space after last allocated block
        if (!used_regions.empty()) {
            char* last_end = used_regions.back().second;
            if (end_ptr - last_end >= size) {
                return last_end;
            }
        }
        
        // No space found
        return nullptr;
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
        
        // Memory Manager functions

        // Create
        int create(size_t size, const std::string& type) {
            
            // Find free memory
            void* block_address = find_free_memory(size);
            if (!block_address) {
                block_address = find_free_memory(size);
                
                if (!block_address) {
                    std::cerr << "Out of memory" << std::endl;
                    return -1;
                }
            }

            // Create memory block
            int block_id = next_block_id++;
            allocated_blocks[block_id] = {
                block_address,
                size,
                type,
                1,  // Initial reference count is 1
                false  // Don't free
            };
            
            std::cout << "Created block " << block_id << " of size " << size << " bytes, type " << type << std::endl;
            
            memory_dump();
            return block_id;
        };

    // Set
    bool set(int id, const void* value, size_t value_size) {        
        if (allocated_blocks.find(id) == allocated_blocks.end() || 
            allocated_blocks[id].is_free) {
            std::cerr << "Not a block ID: " << id << std::endl;
            return false;
        }
        
        MemoryBlock& block = allocated_blocks[id];
        if (value_size > block.size) {
            std::cerr << "Value not compatible with block " << id << std::endl;
            return false;
        }
        
        // Copy value to memory block
        std::memcpy(block.address, value, value_size);
        
        std::cout << "Set value in block " << id << std::endl;
        
        memory_dump();
        return true;
    }
    
    // Get
    bool get(int id, void* value, size_t value_size) {
        
        if (allocated_blocks.find(id) == allocated_blocks.end() || 
            allocated_blocks[id].is_free) {
            std::cerr << "Not a block ID: " << id << std::endl;
            return false;
        }
        
        MemoryBlock& block = allocated_blocks[id];
        if (value_size > block.size) {
            std::cerr << "Requested size too large for block " << id << std::endl;
            return false;
        }
        
        // Copy value from memory block
        std::memcpy(value, block.address, value_size);
        
        std::cout << "Got value from block" << id << std::endl;
        return true;
    };
};

// class MemoryServiceImpl final : public MemoryService::Service {
// private:
//     MemoryManager* memory_manager;

// public:
//     MemoryServiceImpl(MemoryManager* mem_mgr) : memory_manager(mem_mgr) {}
    
//     grpc::Status Create(grpc::ServerContext* context, 
//                         const memory_manager::CreateRequest* request,
//                         memory_manager::CreateResponse* response) override {
//         int id = memory_manager->create(request->size(), request->type());
//         if (id < 0) {
//             return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Out of memory");
//         }
//         response->set_id(id);
//         return grpc::Status::OK;
//     };
// };

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
    
    //MemoryServiceImpl service(&memory_manager);

    // Create gRPC server
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    //builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> mem_mgr(builder.BuildAndStart());
    
    // Print results
    std::cout << "Server listening on " << server_address << std::endl;
    std::cout << "Memory Manager initialized with " << mem_size << "MB" << std::endl;
    std::cout << "Memory dumps will be stored in " << dump_folder << std::endl;
    
    mem_mgr->Wait();
    return 0;
}