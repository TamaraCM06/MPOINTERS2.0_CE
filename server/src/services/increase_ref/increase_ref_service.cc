#include "increase_ref_service.h"

int increaseRefCount(int id) {
    std::cout << "IncreaseRefCount called with ID: " << id << std::endl;
    return 2; // Example new reference count
}