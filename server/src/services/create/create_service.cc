#include "create_service.h"
#include "../utils.h"
#include <iostream>

int create(MemoryManager* memory_manager, int size, const std::string& type) {
    // Validar el tipo y tamaño
    if (!validate_type_and_size(type, static_cast<size_t>(size))) {
        return -1; // Indicar fallo
    }

    // Delegar la lógica de asignación a MemoryManager
    int id = memory_manager->create(size, type);

    if (id == -1) {
        std::cerr << "Create operation failed: Not enough memory or invalid type." << std::endl;
    } else {
        std::cout << "Create operation successful. ID = " << id << std::endl;
    }

    return id;
}