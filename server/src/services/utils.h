#ifndef UTILS_H
#define UTILS_H

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstring>
#include <stdexcept>

// Map of type names to their sizes
static const std::unordered_map<std::string, size_t> type_sizes = {
    {"int", sizeof(int)},
    {"float", sizeof(float)},
    {"double", sizeof(double)},
    {"char", sizeof(char)},
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

// Function to convert and validate a value
inline bool convert_and_validate(const std::string& type, const std::string& value, void* output, size_t block_size) {
    try {
        if (type == "int") {
            if (block_size < sizeof(int)) {
                throw std::runtime_error("Block size too small for type int");
            }
            int int_value = std::stoi(value);
            std::memcpy(output, &int_value, sizeof(int));
        } else if (type == "float") {
            if (block_size < sizeof(float)) {
                throw std::runtime_error("Block size too small for type float");
            }
            float float_value = std::stof(value);
            std::memcpy(output, &float_value, sizeof(float));
        } else if (type == "double") {
            if (block_size < sizeof(double)) {
                throw std::runtime_error("Block size too small for type double");
            }
            double double_value = std::stod(value);
            std::memcpy(output, &double_value, sizeof(double));
        } else if (type == "char") {
            if (block_size < sizeof(char)) {
                throw std::runtime_error("Block size too small for type char");
            }
            if (value.size() != 1) {
                throw std::runtime_error("Value is not a valid char");
            }
            char char_value = value[0];
            std::memcpy(output, &char_value, sizeof(char));
        } else if (type == "bool") {
            if (block_size < sizeof(bool)) {
                throw std::runtime_error("Block size too small for type bool");
            }
            bool bool_value = (value == "true");
            std::memcpy(output, &bool_value, sizeof(bool));
        } else {
            throw std::runtime_error("Unsupported type: " + type);
        }
    } catch (const std::exception& e) {
        std::cerr << "Conversion failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}

#endif // UTILS_H