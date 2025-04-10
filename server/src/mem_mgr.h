#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include "dumps/dumps.h"

class GarbageCollector;

struct MemoryBlock {
    void* address;
    size_t size;
    std::string type;
    int ref_count;
};

class MemoryManager {
private:
    void* memory_chunk;
    size_t memory_chunk_size;
    size_t memory_offset = 0;
    int next_id = 1;
    std::unordered_map<int, MemoryBlock> allocations;
    Dumps dumps;
    GarbageCollector* garbage_collector = nullptr;

public:
    MemoryManager(size_t size_mb, const std::string& folder);
    ~MemoryManager();

    void* get_memory_chunk() const;
    size_t get_memory_chunk_size() const;
    size_t get_memory_offset() const;
    size_t get_allocations_count() const;
    int get_next_id() const;

    void update_dumps();
    void log_memory_state();

    int create(int size, const std::string& type);
    bool set(int id, const std::string& value);
    std::string get(int id);
    int increaseRefCount(int id);
    int decreaseRefCount(int id);

    void deallocate(int id);
    void defragment();

    void set_garbage_collector(GarbageCollector* gc) {
        garbage_collector = gc;
    }

    const std::unordered_map<int, MemoryBlock>& get_allocations() const {
        return allocations;
    }
};

#endif // MEM_MGR_H