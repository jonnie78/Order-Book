#include"Order-Book.h"

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