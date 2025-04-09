#include "increase_ref_service.h"

int increaseRefCount(MemoryManager* memory_manager, int id) {
    std::cout << "IncreaseRefCount called with ID: " << id << std::endl;
    return memory_manager->increaseRefCount(id);
}