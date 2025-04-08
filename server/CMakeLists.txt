# Cmake, gRPC and Protobuf Set Up // Don't Edit
#####################################################################################
cmake_minimum_required(VERSION 3.20)
project(myserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

#####################################################################################

# Ruta de los .proto
set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/hello.proto)

# Directorio donde se generarán los archivos
set(GENERATED_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

# Obtener ubicación del plugin de gRPC
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

# Generar código C++ de Protobuf
protobuf_generate(
    LANGUAGE cpp
    OUT_VAR PROTO_SRCS
    PROTOS ${PROTO_FILES}
    PROTOC_OUT_DIR ${GENERATED_PROTO_DIR}
)

# Generar código gRPC
protobuf_generate(
    LANGUAGE grpc
    OUT_VAR GRPC_SRCS
    PROTOS ${PROTO_FILES}
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PROTOC_OUT_DIR ${GENERATED_PROTO_DIR}
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

# Crear la librería con los archivos generados
add_library(protolib ${PROTO_SRCS} ${GRPC_SRCS})

# Incluir encabezados generados
target_include_directories(protolib PUBLIC ${GENERATED_PROTO_DIR})

# Enlazar con las librerías de protobuf y grpc
target_link_libraries(protolib gRPC::grpc++ protobuf::libprotobuf)


# Execute Server and Client // Don't Edit
add_executable(mem_mgr 
    src/mem_mgr.cc 
    src/dumps/dumps.cc
    src/services/create/create_service.cc
    src/services/set/set_service.cc
    src/services/get/get_service.cc
    src/services/increase_ref/increase_ref_service.cc
    src/services/decrease_ref/decrease_ref_service.cc
)
target_include_directories(mem_mgr PRIVATE src/dumps)
target_link_libraries(mem_mgr protolib)

add_executable(client src/client.cc src/parsing/parsing.cc)
target_include_directories(client PRIVATE src/parsing)
target_link_libraries(client protolib)