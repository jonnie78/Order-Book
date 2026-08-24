//Ring buffer template

#pragma once

#include<atomic>
#include<vector>
#include <cstddef>

template <typename T> class SPSC {
private:
    //Buffer vector
    std::vector<T> buffer;
    //Size of buffer
    const size_t capacity;
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
public:
    //Constructor
    SPSC(size_t size) 
    : buffer(size +1),
      capacity(size +1),
      head(0),
      tail(0) {}
    //Function to push data to consumer thread (Orderbook & GUI)
    bool push(const T& order_data) {
        const size_t current_tail = tail.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) % capacity;

        //Checking if space is available for consumer
        if (next_tail == head.load(std::memory_order_acquire)) {
            return false;
        }

        buffer[current_tail] = order_data;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }
    //Function to read data from producer thread (Generator & Orderbook)
    bool pop(T& result) {
        const size_t current_head = head.load(std::memory_order_relaxed);

        //Check if data is ready from producer
        if (current_head == tail.load(std::memory_order_acquire)) {
            return false;
        }

        result = buffer[current_head];
        const size_t next_head = (current_head + 1) % capacity;
        head.store(next_head, std::memory_order_release);
        return true;
    }
    //Copy constructor
    SPSC(const SPSC&) = delete;
    //Copy assignment
    SPSC& operator=(const SPSC&) = delete;
    //Move constructor
    SPSC(SPSC&&) = delete;
    //Move assignment
    SPSC& operator=(SPSC&&) = delete;
};