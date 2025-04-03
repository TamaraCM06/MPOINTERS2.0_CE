#include <grpcpp/grpcpp.h>
#include <iostream>
#include "proto/hello.grpc.pb.h"
#include "proto/hello.pb.h"

int main() {
    // Server address
    std::string server_address("0.0.0.0:9999");

    // Create a channel to connect to the server
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());

    // Create a stub for the ProcessingServices service
    std::unique_ptr<ProcessingServices::Stub> stub = ProcessingServices::NewStub(channel);

    // Create a Point3 request
    Point3 request;
    request.set_x(1.0);
    request.set_y(2.0);
    request.set_z(3.0);

    // Prepare the response
    Numeric response;

    // Create a gRPC context
    grpc::ClientContext context;

    // Call the computeSum RPC
    grpc::Status status = stub->computeSum(&context, request, &response);

    // Check the status and print the result
    if (status.ok()) {
        std::cout << "Sum: " << response.value() << std::endl;
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
    }

    return 0;
}