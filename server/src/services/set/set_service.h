#ifndef SET_SERVICE_H
#define SET_SERVICE_H

#include <string>
#include <iostream>
#include "../mem_mgr.h"

bool set(MemoryManager* memory_manager, int id, const std::string& value);

#endif // SET_SERVICE_H