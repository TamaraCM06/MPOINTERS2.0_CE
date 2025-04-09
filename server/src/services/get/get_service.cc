#include "get_service.h"

std::string get(MemoryManager* memory_manager, int id) {
    std::cout << "Get called with ID: " << id << std::endl;

    // Delegate the logic to MemoryManager
    return memory_manager->get(id);
}