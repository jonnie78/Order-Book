//Memory pool class template

#pragma once

#include<cstdint>

template <typename T> class MemoryPool {
private:
    uint32_t num_blocks;
    uint32_t block_size;
    uint32_t num_free_blocks;
    uint32_t num_initialised;
    unsigned char* memory_start;
    unsigned char* next;
public:
    //Constructor
    MemoryPool(uint32_t blocks) {
        num_blocks = blocks;
        num_free_blocks = num_blocks;
        num_initialised = 0;
        memory_start = new unsigned char [sizeof(T) * num_blocks];
        next = memory_start;
    }
    //Destructor
    ~MemoryPool() { 
        delete[] memory_start;
        memory_start = nullptr;
    }
    //Address to index 
    unsigned char* address_from_index(uint32_t i) const {
        return memory_start + (i * sizeof(T));
    }
    //Index to address
    uint32_t index_from_address(const unsigned char* p) const {
        return ((static_cast<uint32_t>(p - memory_start)) / sizeof(T)); 
    }
    //Allocate function
    T* allocate() {
        if (num_initialised < num_blocks) {
            uint32_t* p = reinterpret_cast<uint32_t*>(address_from_index(num_initialised));
            *p = num_initialised + 1;
            num_initialised++;
        }
        T* value = nullptr;
        if (num_free_blocks > 0) {
            value = reinterpret_cast<T*>(next);
            --num_free_blocks;
            if (num_free_blocks != 0) {
                next = address_from_index(*reinterpret_cast<uint32_t*>(next));
            }
            else {
                next = nullptr;
            }
        }
        return value;
    }
    //De-Allocate function
    void de_allocate(T* p) {
        if (next != nullptr) {
            (*reinterpret_cast<uint32_t*>(p)) = index_from_address(next);
            next = reinterpret_cast<unsigned char*>(p);
        }
        else {
            *(reinterpret_cast<uint32_t*>(p)) = num_blocks;
            next = reinterpret_cast<unsigned char*>(p);
        }
        ++num_free_blocks;
    }
    //Copy constructor
    MemoryPool(const MemoryPool&) = delete;
    //Copy assignment
    MemoryPool& operator=(const MemoryPool&) = delete;
    //Move constructor
    MemoryPool(MemoryPool&&) = delete;
    //Move assignment
    MemoryPool& operator=(MemoryPool&&) = delete;
};