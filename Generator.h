//Order generator
#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>
#include <chrono>
#include <cstdint>
#include"Structs.h"

class Generator {
private:
    //RNG
    std::mt19937_64 rng;
    //Arrivale rate distribution(poisson dist)
    std::exponential_distribution<double> arrival_rate;
    //Client ID distrivution 
    std::uniform_int_distribution<uint32_t> client_dist;
    //Quantity distribution
    std::uniform_int_distribution<uint32_t> quantity_dist;
    //Price offset distribution (exponential to be clustered around reference price)
    std::exponential_distribution<uint32_t> price_offset_dist;
    //Bid or ask  distrivution
    std::bernoulli_distribution type_dist;
    //Order ID tracker
    uint64_t order_id_tracker;
    //Holder for price reference
    double ref;
public:
    //Constructor, order rate per sec
    Generator(double order_rate, uint32_t num_clients, uint32_t min_quantity, uint32_t max_quantity, double reference_price, double mean_price_offset);
    //Function to generate orders
    Order generate_order();
    //Function to hold order wait time generation
    double wait_time();
};

#endif