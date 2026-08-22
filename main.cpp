#include "Generator.h"
#include "Order-Book.h"
#include "Ring-Buffer.h"
#include <thread>
#include <atomic>

std::atomic<bool> running{true};

void generator_thread_func(Generator& gen, SPSC<Order>& buffer) {
    while (running) {
        double wait_us = gen.wait_time();
        std::this_thread::sleep_for(std::chrono::microseconds((long long)wait_us));
        Order o = gen.generate_order();
        while (!buffer.push(o) && running) {
        }
    }
}

int main() {
    Generator gen(1000.0, 50, 1, 100, 100.0, 1.0); // order_rate, clients, min/max qty, ref price, stddev
    SPSC<Order> gen_to_book(1024);
    OrderBook book(100000);

    std::thread producer(generator_thread_func, std::ref(gen), std::ref(gen_to_book));

    Order received;
    int count = 0;
    while (count < 20) {
        if (gen_to_book.pop(received)) {
            book.add_order(received);
            book.print_book();
            count++;
        }
    }

    running = false;
    producer.join();
}