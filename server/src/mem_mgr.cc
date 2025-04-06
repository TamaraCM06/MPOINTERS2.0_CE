#include <grpcpp/grpcpp.h>
#include "proto/hello.grpc.pb.h"
#include "proto/hello.pb.h"


class ProcessingImpl final : public ProcessingServices::Service {
    public:
    ~ProcessingImpl() noexcept override = default;
    grpc::Status computeSum(
        ::grpc::ServerContext* context, const Point2* request, Numeric* response) override{
        std::cout << "Called!!!" << std::endl;
        response->set_value(request->x() + request->y() + request->z());

        return grpc::Status::OK;
        }
};

int main(){
    std::string server_address("0.0.0.0:9999");
    ProcessingImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
    return 0;
}