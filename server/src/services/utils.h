#ifndef UTILS_H
#define UTILS_H

#include <unordered_map>
#include <string>
#include <iostream>

// Map of type names to their sizes
static const std::unordered_map<std::string, size_t> type_sizes = {
    {"int", sizeof(int)},
    {"float", sizeof(float)},
    {"double", sizeof(double)},
    {"char", sizeof(char)},
    {"short", sizeof(short)},
    {"long", sizeof(long)},
    {"long long", sizeof(long long)},
    {"bool", sizeof(bool)}
};

// Function to validate the type and size
inline bool validate_type_and_size(const std::string& type, size_t size) {
    auto it = type_sizes.find(type);
    if (it == type_sizes.end()) {
        std::cerr << "Invalid type: " << type << std::endl;
        return false; // Type is not recognized
    }

    if (it->second != size) {
        std::cerr << "Size mismatch for type " << type << ": expected " << it->second
                  << " bytes, got " << size << " bytes" << std::endl;
        return false; // Size does not match the expected size for the type
    }

    return true; // Type and size are valid
}

#endif // UTILS_H