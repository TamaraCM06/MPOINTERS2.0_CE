#include "get_service.h"

std::string get(int id) {
    std::cout << "Get called with ID: " << id << std::endl;
    return "example_value"; // Example value
}