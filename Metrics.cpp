#include <algorithm>
#include"Metrics.h"

using namespace std;

//Constructor 
Metrics::Metrics() {
    orders_processed = 0;
    total_volume = 0;
    last_reset = chrono::steady_clock::now();
}
//Recordering total orders processed
void Metrics::record_order_received() {
    orders_processed += 1;
}
//Recording total volume and each order latency
void Metrics::record_trade(uint32_t quantity, uint64_t latency_ns) {
    total_volume += quantity;
    latency_samples[sample_index] = latency_ns;
    sample_index = (sample_index + 1) % latency_samples.size();
}
//Calculating and returning throughput per second
double Metrics::get_throughput() {
   auto now = chrono::steady_clock::now();
   double elapsed_sec = chrono::duration<double>(now - last_reset).count();
   uint64_t count = orders_processed.exchange(0);  //Reads and reset atomically
   last_reset = now;
   return elapsed_sec > 0 ? count / elapsed_sec : 0.0;
}
//Returning total traded volume
 uint64_t Metrics::get_total_volume() const {
    return total_volume;
 }
 //Calculating and returning 50th percentile of latency
 double Metrics::latency_p50() const {
    array<uint64_t, 4096> sorted_copy = latency_samples;
    sort(sorted_copy.begin(), sorted_copy.end());
    return static_cast<double>(sorted_copy[static_cast<size_t>(4096 * 0.5)]); //50% of N (array sie)
 }
  //Calculating and returning 99th percentile of latency
 double Metrics::latency_p99() const {
    array<uint64_t, 4096> sorted_copy = latency_samples;
    sort(sorted_copy.begin(), sorted_copy.end());
    return static_cast<double>(sorted_copy[static_cast<size_t>(4096 * 0.99)]); //99% of N (array sie)
 }