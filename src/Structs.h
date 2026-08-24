#pragma once

#include<cstdint>

//Struct to carry order data
struct Order {
    //Order data members, uint used to guarentee memory size, beneficial for memory pool
    uint64_t order_id;
    uint64_t timestamp; //nanoseconds
    uint32_t client_id;
    uint32_t quantity;
    uint32_t price;
    bool is_buy;

    //Pointers for intrusive linked list
    Order* next = nullptr;
    Order* prev = nullptr;
};

//Struct for holding orders at each price level
struct PriceLevel {
    Order* head = nullptr;
    Order* tail = nullptr;
};
//Placeholder for data analytics snapshot struct
//Stuct for holding price level volumes
struct PriceVolume {
    uint32_t price;
    uint32_t volume;
};
//Data snapshot struct
struct Snapshot {
    //From orderbook
    std::vector<PriceVolume> bid_levels;
    std::vector<PriceVolume> ask_levels;
    Order recent_orders[20];
    int recent_count = 0;

    //From metrics
    uint64_t throughput;
    uint64_t total_volume;
    double latency_p50;
    double latency_p99;
};