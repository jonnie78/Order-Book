#include"Generator.h"

using namespace std;

//Constructor
Generator::Generator(double order_rate, uint32_t num_clients, uint32_t min_quantity, uint32_t max_quantity, double reference_price, double price_stddev) 
    : rng(374), //can change to random seed
      arrival_rate(order_rate / 1e6),
      client_dist(1, num_clients),
      quantity_dist(min_quantity, max_quantity),
      price_offset_dist(0, price_stddev),
      type_dist(0.5){
        order_id_tracker = 1;
        ref = reference_price;
      } 
//Generator function
Order Generator::generate_order() {
    bool type = type_dist(rng);
    return Order {
        .order_id = order_id_tracker++,
        .timestamp = static_cast<uint64_t>(
            chrono::duration_cast<chrono::nanoseconds>(
                chrono::steady_clock::now().time_since_epoch()
            ).count()),
        .client_id = client_dist(rng),
        .quantity = quantity_dist(rng),
        .price = static_cast<uint32_t>(max(0.0, ref + price_offset_dist(rng))),
        .is_buy = type
    };
}
//Function to hold order wait time generation
double Generator::wait_time() {
    return arrival_rate(rng);
}