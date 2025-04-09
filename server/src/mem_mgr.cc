#include <grpcpp/grpcpp.h>
#include "proto/hello.grpc.pb.h"
#include "proto/hello.pb.h"
#include <iostream>
#include <string>
#include <getopt.h>
#include <cstdlib>
#include <unordered_map>
#include <cstring>
#include "dumps/dumps.h"
#include "mem_mgr.h"
#include "services/create/create_service.h"
#include "services/set/set_service.h"
#include "services/get/get_service.h"
#include "services/increase_ref/increase_ref_service.h"
#include "services/decrease_ref/decrease_ref_service.h"
#include "services/utils.h"

MemoryManager::MemoryManager(size_t size_mb, const std::string& folder)
    : memory_chunk_size(size_mb * 1024 * 1024),
      dumps(folder, memory_chunk_size) {
    memory_chunk = malloc(memory_chunk_size);
    if (!memory_chunk) {
        std::cerr << "Failed to allocate " << size_mb << "MB of memory" << std::endl;
        exit(1);
    }
    std::cout << "Allocated " << size_mb << "MB of memory" << std::endl;
    dumps.update(0, memory_chunk_size, 0, next_id); // Initialize the dump file
}

MemoryManager::~MemoryManager() {
    if (memory_chunk) {
        free(memory_chunk);
    }
}

void* MemoryManager::get_memory_chunk() const {
    return memory_chunk;
}

size_t MemoryManager::get_memory_chunk_size() const {
    return memory_chunk_size;
}

void MemoryManager::update_dumps() {
    size_t used_memory = memory_offset;
    size_t free_memory = memory_chunk_size - memory_offset;
    dumps.update(used_memory, free_memory, allocations.size(), next_id);

    // Debugging output
    std::cout << "Debug: used_memory = " << used_memory << std::endl;
    std::cout << "Debug: free_memory = " << free_memory << std::endl;
    std::cout << "Debug: allocated_blocks = " << allocations.size() << std::endl;
    std::cout << "Debug: next_id = " << next_id << std::endl;
}

int MemoryManager::create(int size, const std::string& type) {
    // Verificar si hay suficiente espacio
    if (memory_offset + static_cast<size_t>(size) > memory_chunk_size) {
        std::cerr << "Not enough memory to allocate " << size << " bytes" << std::endl;
        return -1;
    }

    // Asignar memoria
    void* block_address = static_cast<char*>(memory_chunk) + memory_offset;

    // Crear un nuevo bloque de memoria
    MemoryBlock block = {
        .address = block_address,
        .size = static_cast<size_t>(size),
        .type = type,
        .ref_count = 1
    };

    // Almacenar el bloque en el mapa de asignaciones
    int id = next_id++;
    allocations[id] = block;

    // Actualizar el desplazamiento de memoria
    memory_offset += static_cast<size_t>(size);

    std::cout << "Allocated " << size << " bytes for type " << type << " with ID " << id << std::endl;

    // Actualizar el archivo de volcado
    update_dumps();

    return id; // Retornar el ID único
}


bool MemoryManager::set(int id, const std::string& value) {
    auto it = allocations.find(id);
    if (it == allocations.end()) {
        std::cerr << "Set failed: ID " << id << " not found." << std::endl;
        return false; // Retornar false si el ID no existe
    }

    MemoryBlock& block = it->second;

    // Validar y convertir el valor
    if (!convert_and_validate(block.type, value, block.address, block.size)) {
        std::cerr << "Set failed: Conversion or validation failed for ID " << id << "." << std::endl;
        return false;
    }

    std::cout << "Set successful for ID " << id << ": " << value << std::endl;
    return true;
}

// Dummy functions for other services

std::string MemoryManager::get(int id) {
    std::cout << "Get operation is not implemented yet." << std::endl;
    return "";
}

int MemoryManager::increaseRefCount(int id) {
    std::cout << "IncreaseRefCount operation is not implemented yet." << std::endl;
    return -1;
}

int MemoryManager::decreaseRefCount(int id) {
    std::cout << "DecreaseRefCount operation is not implemented yet." << std::endl;
    return -1;
}

class MemoryManagerServiceImpl final : public memory_manager::MemoryManager::Service {
private:
    MemoryManager* memory_manager;

public:
    explicit MemoryManagerServiceImpl(MemoryManager* mem_mgr) : memory_manager(mem_mgr) {}

    grpc::Status Create(::grpc::ServerContext* context, const memory_manager::CreateRequest* request,
                        memory_manager::CreateResponse* response) override {
        int id = memory_manager->create(request->size(), request->type());
        if (id == -1) {
            response->set_id(id);
            response->set_success(false); // Mark the operation as failed
            response->set_message("Create operation failed. Invalid type or insufficient memory.");
        } else {
            response->set_id(id);
            response->set_success(true); // Mark the operation as successful
            response->set_message("Create operation successful.");
        }
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