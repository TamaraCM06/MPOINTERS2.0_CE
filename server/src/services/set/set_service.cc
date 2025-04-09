#include "set_service.h"

bool set(MemoryManager* memory_manager, int id, const std::string& value) {
    std::cout << "Set called with ID: " << id << " and value: " << value << std::endl;

    // Delegar la lógica de validación y escritura a MemoryManager
    return memory_manager->set(id, value);
}