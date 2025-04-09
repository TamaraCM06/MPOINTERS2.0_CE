#include "decrease_ref_service.h"

int decreaseRefCount(MemoryManager* memory_manager, int id) {
    std::cout << "DecreaseRefCount called with ID: " << id << std::endl;
    return memory_manager->decreaseRefCount(id);
}