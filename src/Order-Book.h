#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include<map>
#include<unordered_map>
#include<functional>
#include"Structs.h"      
#include"Memory-Pool.h"
#include"Metrics.h"

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
    //Metrics
    Metrics metrics;
    //Add order to its price levels linked list
    void add_to_level(PriceLevel& price_level, Order* order);
    //Remove order from its price levels linked list
    void pop_from_level(PriceLevel& price_level);
public:
    //Constructor
    OrderBook(uint32_t order_capacity);
    //Function to add orders to memory pool
    void add_order(const Order& incoming_order);
    //Function to gather price level depth
    void depth_levels(std::vector<PriceVolume>& bid_depth, std::vector<PriceVolume>& ask_depth);
    //Function to extract 20 recent orders
    void get_recent_orders(Order out[20], int& out_count) const;
    //Getter for metrics
    Metrics& get_metrics() { return metrics; }
    //Helper function to get time
    static uint64_t now_ns();
private:
//Function template for order matching function
template <typename OpposingType, typename PriceComparison>
void order_match(Order* incoming_order, OpposingType& opposing_side, PriceComparison condition) {
    auto tracker = opposing_side.begin(); //Tracker to act as holder of what price order is compared against
    while (tracker != opposing_side.end() && incoming_order->quantity > 0) {
        PriceLevel& price_level = tracker->second;

        if (!condition(incoming_order->price, tracker->first)) break; //Breaks if lambda condition isnt met

        while (price_level.head && incoming_order->quantity > 0) { //Loop that executes trades
            Order* resting_order = price_level.head;
            uint32_t traded_quantity = std::min(incoming_order->quantity, resting_order->quantity);
            incoming_order->quantity -= traded_quantity;
            resting_order->quantity -= traded_quantity;

            metrics.record_trade(traded_quantity, now_ns() - incoming_order->timestamp);

            if (resting_order->quantity == 0) { //Erasing resting orders once their quantity has been depleted
                order_lookup.erase(resting_order->order_id);
                pop_from_level(price_level);
                memory_pool.de_allocate(resting_order);
            }
        }
        if (!price_level.head) { //Once all orders have been filled at certain price level, its erased
            tracker = opposing_side.erase(tracker);
        }
        else {
            ++tracker;
        }
    }
}
};

#endif