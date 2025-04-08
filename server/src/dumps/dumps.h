#ifndef DUMPS_H
#define DUMPS_H

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

class Dumps {
private:
    std::string dump_folder;
    std::string base_chunk_file;
    size_t memory_chunk_size;

public:
    Dumps(const std::string& folder, size_t memory_size);
    void initialize_base_chunk();
};

#endif // DUMPS_H