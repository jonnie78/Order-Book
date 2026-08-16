#pragma once

#include<cstdint>

//Struct to carry order data
struct Order {
    //Order data members, uint used to guarentee memory size, beneficial for memory pool
    uint64_t order_id;
    uint64_t timestamp;
    uint32_t client_id;
    uint32_t quantity;
    uint32_t price;
    bool is_buy;

    //Pointers for intrusive linked list
    Order* next = nullptr;
    Order* prev = nullptr;
};

//Placeholder for data analytics snapshot struct