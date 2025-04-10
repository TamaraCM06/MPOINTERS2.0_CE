#ifndef GARBAGE_COLLECTOR_H

#include <thread>
#include <iostream>
#include <condition_variable>
#include <functional>
#include "../mem_mgr.h"
#include <atomic>
#include <mutex>
#include <queue>

class GarbageCollector {
public:
    explicit GarbageCollector(MemoryManager* memory_manager);
    ~GarbageCollector();

    void notify(int id);

    void start();
    void stop();

    MemoryManager* memory_manager;

private:
    void garbage_collection();

    std::thread gc_thread;

    // Mutex for thread aka anti-collisions
    std::mutex mutex;

    std::queue<int> to_collect;
    // Condition variable for signaling the garbage collector
    std::condition_variable cv;
    
    std::atomic<bool> should_stop;

    // Flag to indicate if the garbage collector is running
    std::atomic<bool> is_running;

};

#endif // GARBAGE_COLLECTOR_H