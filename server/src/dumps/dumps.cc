#include "dumps.h"

Dumps::Dumps(const std::string& folder, size_t memory_size)
    : dump_folder(folder), memory_chunk_size(memory_size) {
    base_chunk_file = dump_folder + "/Base_chunk.txt";

    // Create the dumps folder if it doesn't exist
    std::filesystem::create_directories(dump_folder);

    // Initialize the Base_chunk.txt file
    initialize_base_chunk();
}

void Dumps::initialize_base_chunk() {
    if (std::filesystem::exists(base_chunk_file)) {
        // File already exists, load its content
        std::ifstream file(base_chunk_file);
        if (file.is_open()) {
            std::cout << "Base_chunk.txt already exists. Content:" << std::endl;
            std::string line;
            while (std::getline(file, line)) {
                std::cout << line << std::endl;
            }
            file.close();
        } else {
            throw std::runtime_error("Failed to open Base_chunk.txt for reading.");
        }
    } else {
        // Create the initial Base_chunk.txt file
        std::ofstream file(base_chunk_file);
        if (file.is_open()) {
            file << "{\n";
            file << "  \"memory_size\": " << memory_chunk_size << ",\n";
            file << "  \"used_memory\": 0,\n";
            file << "  \"free_memory\": " << memory_chunk_size << ",\n";
            file << "  \"allocated_blocks\": 0,\n";
            file << "  \"next_available_id\": 1\n";
            file << "}\n";
            file.close();
            std::cout << "Created Base_chunk.txt with initial structure." << std::endl;
        } else {
            throw std::runtime_error("Failed to create Base_chunk.txt.");
        }
    }
}