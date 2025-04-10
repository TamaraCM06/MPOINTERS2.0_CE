#ifndef DUMPS_H
#define DUMPS_H

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <sstream>

class Dumps {
private:
    std::string dump_folder;
    std::string base_chunk_file;
    size_t memory_chunk_size;

public:
    Dumps(const std::string& folder, size_t memory_size);
    void initialize_base_chunk();
    void update(size_t used_memory, size_t free_memory, int allocated_blocks, int next_id);
    void create_detailed_dump_file(const std::string& formatted_memory_blocks);
};

#endif // DUMPS_H