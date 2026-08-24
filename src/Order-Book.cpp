#include"Order-Book.h"
#include<iostream>
#include <chrono>

using namespace std;

//Constructor
OrderBook::OrderBook(uint32_t order_capacity)
    : memory_pool(order_capacity) {}

//Add order to its price levels linked list
void OrderBook::add_to_level(PriceLevel& price_level, Order* order) {
    order->prev = price_level.tail;
    order->next = nullptr;
    if (price_level.tail) price_level.tail->next = order;
    price_level.tail = order;
    if (!price_level.head) price_level.head = order;
}
//Remove order from its price levels linked list
void OrderBook::pop_from_level(PriceLevel& price_level) {
    Order* front = price_level.head;
    price_level.head = front->next;
    if (price_level.head) price_level.head->prev = nullptr;
    else price_level.tail = nullptr;
}
//Add order function
void OrderBook::add_order(const Order& incoming_order) {
    Order* new_order = memory_pool.allocate();
    if (!new_order) return; //Memory pool exhausted
    new_order->order_id = incoming_order.order_id;
    new_order->client_id = incoming_order.client_id;
    new_order->quantity = incoming_order.quantity;
    new_order->price = incoming_order.price;
    new_order->is_buy = incoming_order.is_buy;
    new_order->timestamp = incoming_order.timestamp;
    new_order->next = new_order->prev = nullptr;
    metrics.record_order_received();

    //Matching to opposing side
    if (new_order->is_buy) {
        order_match(new_order, asks, 
            [] (uint32_t bid, uint32_t ask) {return bid >= ask; });
    }
    else {
        order_match(new_order, bids,
            [] (uint32_t ask, uint32_t bid) {return ask <= bid; });
    }

    //Managing leftover quantity
    if (new_order->quantity > 0) {
        if (new_order->is_buy) {
            PriceLevel& price_level = bids[new_order->price];
            add_to_level(price_level, new_order);
        }
        else {
            PriceLevel& price_level = asks[new_order->price];
            add_to_level(price_level, new_order);
        }
    order_lookup[new_order->order_id] = new_order;
    }
    else {
        memory_pool.de_allocate(new_order);
    }
}
//Function to create depth charts for snapshot
void OrderBook::depth_levels(vector<PriceVolume>& bid_depth, vector<PriceVolume>& ask_depth) {
    bid_depth.clear();
    uint32_t cumulative = 0;
    for (const auto& [price, level] : bids) {
        uint32_t total = 0;
        Order* o = level.head;
        while(o) {
            total += o->quantity;
            o = o->next;
        }
        cumulative += total;
        bid_depth.push_back({price, cumulative});
    }

    ask_depth.clear();
    cumulative = 0;
    for (const auto& [price, level] : asks) {
        uint32_t total = 0;
        Order* o = level.head;
        while(o) {
            total += o->quantity;
            o = o->next;
        }
        cumulative += total;
        ask_depth.push_back({price, cumulative});
    }
}
//Function to extract 20 recent orders
void OrderBook::get_recent_orders(Order out[20], int& out_count) const {
    out_count = 0;
    auto bid_it = bids.begin();
    auto ask_it = asks.begin();

    while (out_count < 20 && (bid_it != bids.end() || ask_it != asks.end())) {
        if (bid_it != bids.end()) {
            Order* o = bid_it->second.head;
            while (o && out_count < 20) { out[out_count++] = *o; o = o->next; }
            ++bid_it;
        }
        if (ask_it != asks.end() && out_count < 20) {
            Order* o = ask_it->second.head;
            while (o && out_count < 20) { out[out_count++] = *o; o = o->next; }
            ++ask_it;
        }
    }
}
//Get time helper function 
uint64_t OrderBook::now_ns() {
    return static_cast<uint64_t>(
        chrono::duration_cast<chrono::nanoseconds>(
            chrono::steady_clock::now().time_since_epoch()
        ).count()
    );
}
//Print function for testing
void OrderBook::print_book() const {
    std::cout << "----- ASKS (low to high) -----\n";
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        std::cout << "Price: " << it->first << " | ";
        Order* o = it->second.head;
        while (o) {
            std::cout << "[id:" << o->order_id << " qty:" << o->quantity << "] ";
            o = o->next;
        }
        std::cout << "\n";
    }
    std::cout << "----- BIDS (high to low) -----\n";
    for (auto it = bids.begin(); it != bids.end(); ++it) {
        std::cout << "Price: " << it->first << " | ";
        Order* o = it->second.head;
        while (o) {
            std::cout << "[id:" << o->order_id << " qty:" << o->quantity << "] ";
            o = o->next;
        }
        std::cout << "\n";
    }
    std::cout<<endl;
}