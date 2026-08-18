#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include<map>
#include<unordered_map>
#include<functional>
#include"Structs.h"      
#include"Memory-Pool.h"

class OrderBook {
private:
    //Memory pool
    MemoryPool<Order> memory_pool;
    //Ordered map to hold ask price levels
    std::map<uint32_t, PriceLevel> asks;
    //Ordered map to hold bid price levels
    std::map<uint32_t, PriceLevel, std::greater<uint32_t>> bids;
    //Unorderd map for order lookup by ID
    std::unordered_map<uint64_t, Order*> order_lookup;
    //Add order to its price levels linked list
    void add_to_level(PriceLevel& price_level, Order* order);
    //Remove order from its price levels linked list
    void pop_from_level(PriceLevel& price_level);
public:
    //Constructor
    OrderBook(uint32_t order_capacity);
    //Function to add orders to memory pool
    void add_order(const Order& incoming_order);
};

#endif