#ifndef CREATE_SERVICE_H
#define CREATE_SERVICE_H

#include <string>
#include "../../mem_mgr.h"

int create(MemoryManager* memory_manager, int size, const std::string& type);

#endif // CREATE_SERVICE_H