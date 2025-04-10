#include "garbage_collector.h"
#include <iostream>
#include "../mem_mgr.h"

GarbageCollector::GarbageCollector(MemoryManager* memory_manager)
    : memory_manager(memory_manager), should_stop(false), is_running(false) {
}

GarbageCollector::~GarbageCollector() {
    stop();
}

void GarbageCollector::start() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!is_running) {
        should_stop = false;
        gc_thread = std::thread(&GarbageCollector::garbage_collection, this);
        is_running = true;
        std::cout << "Garbage collector started" << std::endl;
    }
}

void GarbageCollector::stop(){
    std::lock_guard<std::mutex> lock(mutex);
    if (is_running) {
        should_stop = true;
    }
    cv.notify_one();

    if (gc_thread.joinable()) {
        gc_thread.join();
    }
    is_running = false;

    std::cout <<"Garbage Collector stopped"<< std::endl;
}

void GarbageCollector::notify(int id) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        to_collect.push(id);
        std::cout << "Object " << id << " up for garbage collection" << std::endl;
    }
    
    // Wake up garbage collector to garbage collect
    cv.notify_one();
}

void GarbageCollector::garbage_collection(){
    std::cout << "Garbage collector started" << std::endl;

    while (!should_stop) {
        std::unique_lock<std::mutex> lock(mutex);
        
        // Wait until there's id to collect or asked to stop
        cv.wait(lock, [this] { 
            return !to_collect.empty() || should_stop; 
        });
        
        // If we're asked to stop and the queue is empty, exit
        if (should_stop && to_collect.empty()) {
            break;
        }

        while(!to_collect.empty()){
            int id = to_collect.front();
            to_collect.pop();

            // Perform garbage collection
            std::cout << "Collecting object " << id << std::endl;

            lock.unlock();

            auto& allocations = memory_manager->get_allocations();
            auto it = allocations.find(id);

            if (it != allocations.end() && it->second.ref_count == 0) {
                std::cout << "Garbage collecting object " << id << " of type " 
                          << it->second.type << " with size " << it->second.size << std::endl;

                memory_manager->deallocate(id);
                std::cout << "Memory for object " << id << "ready for defragmentation" << std::endl;
            }
            memory_manager->update_dumps();
            memory_manager->defragment();
            lock.lock();
    
        }
    }
}
