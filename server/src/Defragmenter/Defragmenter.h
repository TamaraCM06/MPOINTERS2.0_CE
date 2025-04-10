#ifndef DEFRAGMENTER_H
#define DEFRAGMENTER_H

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring> // For memmove
#include "mem_mgr.h"

class Defragmenter {
public:
    static void defragment(void* memory_chunk, size_t memory_chunk_size, std::unordered_map<int, MemoryBlock>& allocations, size_t& memory_offset) {
        std::cout << "Starting defragmentation..." << std::endl;

        size_t compact_offset = 0; // Tracks the next free position in memory

        // Create a vector of blocks sorted by their current addresses
        std::vector<std::pair<int, MemoryBlock>> sorted_blocks(allocations.begin(), allocations.end());
        std::sort(sorted_blocks.begin(), sorted_blocks.end(), [](const auto& a, const auto& b) {
            return a.second.address < b.second.address;
        });

        // Process blocks in sorted order
        for (auto& [block_id, block] : sorted_blocks) {
            if (block.ref_count > 0) { // Only move allocated blocks
                void* current_address = block.address;
                void* new_address = reinterpret_cast<char*>(memory_chunk) + compact_offset;

                if (current_address != new_address) {
                    // Move the block to the compact_offset position
                    std::memmove(new_address, current_address, block.size);

                    // Log the movement
                    std::cout << "Block ID " << block_id << " moved from " << current_address << " to " << new_address << std::endl;

                    // Update the block's address
                    block.address = new_address;
                } else {
                    // Log that the block remains in place
                    std::cout << "Block ID " << block_id << " remains at " << current_address << std::endl;
                }

                // Update the compact_offset
                compact_offset += block.size;
            } else {
                // Log that the block is freed
                std::cout << "Block ID " << block_id << " is freed and not moved." << std::endl;
            }
        }

        // Update memory_offset to reflect the new end of allocated memory
        memory_offset = compact_offset;

        std::cout << "Defragmentation complete. Used memory: " << memory_offset
                  << ", Free memory: " << (memory_chunk_size - memory_offset) << std::endl;
    }
};

#endif // DEFRAGMENTER_H