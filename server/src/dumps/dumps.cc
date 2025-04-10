#include "dumps.h"

Dumps::Dumps(const std::string& folder, size_t memory_size)
    : dump_folder(folder), memory_chunk_size(memory_size) {
    // Convert the folder path to an absolute path
    dump_folder = std::filesystem::absolute(dump_folder).string();
    base_chunk_file = dump_folder + "/Base_chunk.txt";

    // Debug: Print the resolved folder path
    std::cout << "Resolved dumps folder path: " << dump_folder << std::endl;

    // Create the dumps folder if it doesn't exist
    if (!std::filesystem::exists(dump_folder)) {
        try {
            std::filesystem::create_directories(dump_folder);
            std::cout << "Created dumps folder at: " << dump_folder << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to create dumps folder: " << e.what() << std::endl;
            throw;
        }
    } else {
        std::cout << "Dumps folder already exists at: " << dump_folder << std::endl;
    }

    // Initialize the Base_chunk.txt file
    initialize_base_chunk();
}

void Dumps::initialize_base_chunk() {
    if (std::filesystem::exists(base_chunk_file)) {
        std::cout << "Base_chunk.txt already exists. Full path: " << base_chunk_file << std::endl;
    } else {
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
            std::cout << "Created Base_chunk.txt at: " << base_chunk_file << std::endl;
        } else {
            std::cerr << "Failed to create Base_chunk.txt at: " << base_chunk_file << std::endl;
            throw std::runtime_error("Failed to create Base_chunk.txt.");
        }
    }
}

void Dumps::update(size_t used_memory, size_t free_memory, int allocated_blocks, int next_id) {
    std::ofstream file(base_chunk_file, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open Base_chunk.txt for writing.");
    }

    file << "{\n";
    file << "  \"memory_size\": " << memory_chunk_size << ",\n";
    file << "  \"used_memory\": " << used_memory << ",\n";
    file << "  \"free_memory\": " << free_memory << ",\n";
    file << "  \"allocated_blocks\": " << allocated_blocks << ",\n";
    file << "  \"next_available_id\": " << next_id << "\n";
    file << "}\n";

    file.close();
    std::cout << "Updated Base_chunk.txt at: " << base_chunk_file << std::endl;
}
void Dumps::create_detailed_dump_file(const std::string& formatted_memory_blocks) {
    // Get the current time and subtract 6 hours
    auto now = std::chrono::system_clock::now() - std::chrono::hours(6);
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Format the timestamp
    std::ostringstream timestamp;
    timestamp << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d_%H:%M:%S")
              << ":" << std::setfill('0') << std::setw(3) << ms.count();

    // Create the dump file name
    std::string filename = dump_folder + "/dump_" + timestamp.str() + ".txt";

    // Open the file for writing
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create dump file: " + filename);
    }

    // Write the memory blocks
    file << "Memory Blocks:\n";
    file << formatted_memory_blocks;

    file.close();
    std::cout << "Created detailed dump file: " << filename << std::endl;
}