#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include "proto/hello.grpc.pb.h"
#include "proto/hello.pb.h"
#include "parsing/parsing.h"

int main(int argc, char* argv[]) {
    // Default server address
    std::string server_address = "0.0.0.0:9999";

    // Check if a custom port is provided as a command-line argument
    if (argc > 1) {
        server_address = "0.0.0.0:" + std::string(argv[1]);
    }

    // Create a channel to connect to the server
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

    // Create a stub for the MemoryManager service
    std::unique_ptr<memory_manager::MemoryManager::Stub> stub = memory_manager::MemoryManager::NewStub(channel);

    while (true) {
        // Read command from the console
        std::string input;
        std::cout << "Enter command (create, set, get, increaseRefCount, decreaseRefCount, or exit): ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        try {
            // Parse the command
            auto [command, args] = CommandParser::parseCommand(input);

            if (command == "create") {
                auto [size, type] = CommandParser::parseCreate(args);

                memory_manager::CreateRequest request;
                request.set_size(size);
                request.set_type(type);

                memory_manager::CreateResponse response;
                grpc::ClientContext context;

                grpc::Status status = stub->Create(&context, request, &response);
                if (status.ok()) {
                    std::cout << "Call returned: ID = " << response.id() << std::endl;
                    std::cout << "Message: " << response.message() << std::endl;
                } else {
                    std::cerr << "Create failed: " << status.error_message() << std::endl;
                }
            } else if (command == "set") {
                auto [id, value] = CommandParser::parseSet(args);

                memory_manager::SetRequest request;
                request.set_id(id);
                request.set_value(value);

                memory_manager::SetResponse response;
                grpc::ClientContext context;

                grpc::Status status = stub->Set(&context, request, &response);
                if (status.ok()) {
                    std::cout << "Message: " << response.message() << std::endl;
                } else {
                    std::cerr << "Set failed: " << status.error_message() << std::endl;
                }
            } else if (command == "get") {
                int id = CommandParser::parseGet(args);

                memory_manager::GetRequest request;
                request.set_id(id);

                memory_manager::GetResponse response;
                grpc::ClientContext context;

                grpc::Status status = stub->Get(&context, request, &response);
                if (status.ok()) {
                    std::cout << "Call returned: Value = " << response.value() << std::endl;
                    std::cout << "Message: " << response.message() << std::endl;
                } else {
                    std::cerr << "Get failed: " << status.error_message() << std::endl;
                }
            } else if (command == "increaseRefCount") {
                int id = CommandParser::parseRefCount(args);

                memory_manager::RefCountRequest request;
                request.set_id(id);

                memory_manager::RefCountResponse response;
                grpc::ClientContext context;

                grpc::Status status = stub->IncreaseRefCount(&context, request, &response);
                if (status.ok()) {
                    std::cout << "Call returned: New RefCount = " << response.new_ref_count() << std::endl;
                    std::cout << "Message: " << response.message() << std::endl;
                } else {
                    std::cerr << "IncreaseRefCount failed: " << status.error_message() << std::endl;
                }
            } else if (command == "decreaseRefCount") {
                int id = CommandParser::parseRefCount(args);

                memory_manager::RefCountRequest request;
                request.set_id(id);

                memory_manager::RefCountResponse response;
                grpc::ClientContext context;

                grpc::Status status = stub->DecreaseRefCount(&context, request, &response);
                if (status.ok()) {
                    std::cout << "Call returned: New RefCount = " << response.new_ref_count() << std::endl;
                    std::cout << "Message: " << response.message() << std::endl;
                } else {
                    std::cerr << "DecreaseRefCount failed: " << status.error_message() << std::endl;
                }
            } else {
                std::cerr << "Unknown command: " << command << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}