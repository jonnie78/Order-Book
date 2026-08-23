#ifndef METRICS_H
#define METRICS_H

#include<cstdint>
#include<atomic>
#include<array>
#include<chrono>


class Metrics {
private:
    std::atomic<uint64_t> orders_processed;
    std::atomic<uint64_t> total_volume;
    //Latency tracking
    std::chrono::steady_clock::time_point last_reset;
    std::array<uint64_t, 4096> latency_samples;
    size_t sample_index = 0;
public:
    //Constructor
    Metrics();
    //Called from OrderBook when an order is received
    void record_order_received();
    //Called from OrderBook's matching loop when a trade executes
    void record_trade(uint32_t quantity, uint64_t latency_ns);
    //Called when building a snapshot to get throughput
    double get_throughput() const;
    //Called when building a snapshot to get total trading volume
    uint64_t get_total_volume() const;
    //Called when building a snapshot to get latency at the 50th percentile
    double latency_p50() const;
    //Called when building a snapshot to get latency at the 99th percentile
    double latency_p99() const;
    //Copy constructor
    Metrics(const Metrics&) = delete;
    //Copy assignment
    Metrics& operator=(const Metrics&) = delete;
    //Move constructor
    Metrics(Metrics&&) = delete;
    //Move assignment
    Metrics& operator=(Metrics&&) = delete;
};

#endif