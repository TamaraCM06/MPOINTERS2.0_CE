#include "set_service.h"

bool set(int id, const std::string& value) {
    std::cout << "Set called with ID: " << id << " and value: " << value << std::endl;
    return true;
}