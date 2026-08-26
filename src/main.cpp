#include "Generator.h"
#include "Order-Book.h"
#include "Ring-Buffer.h"
#include "Structs.h"
#include "Gui.h"
#include <thread>
#include <atomic>
#include <chrono>
#include<iostream>
#include<windows.h>

using namespace std;

atomic<bool> running{true};

//Function to sleep thread between order generations to obtain correct order rate
void precise_wait(double wait_us) {
    auto start = chrono::steady_clock::now();
    auto target = start + chrono::microseconds((long long)wait_us);

    if (wait_us > 20000) { // only bother sleeping if wait is long enough to matter
        this_thread::sleep_for(chrono::microseconds((long long)wait_us - 15000));
    }

    //Busy-wait the remainder for precision
    while (chrono::steady_clock::now() < target) {
    }
}

//Function for generator thread to run
void generator_thread_func(Generator& gen, SPSC<Order>& buffer) {
    while (running) {
        double wait_us = gen.wait_time();
        precise_wait(wait_us);
        Order o = gen.generate_order();
        while (!buffer.push(o) && running) {
        }
    }
}

//Function for backend thread to run (order matching and snapshot creation)
void backend_thread_func(SPSC<Order>& gen_to_book, SPSC<Snapshot>& book_to_gui, OrderBook& book) {
    auto last_snapshot_time = chrono::steady_clock::now();
    const auto snapshot_interval = chrono::milliseconds(50);

    while (running) {
        Order incoming;
        if (gen_to_book.pop(incoming)) {
            book.add_order(incoming);
        }

        auto now = chrono::steady_clock::now();
        if (now - last_snapshot_time >= snapshot_interval) {
            Snapshot snap;
            book.depth_levels(snap.bid_levels, snap.ask_levels);
            book.get_recent_orders(snap.recent_orders, snap.recent_count);

            snap.throughput = book.get_metrics().get_throughput();
            snap.total_volume = book.get_metrics().get_total_volume();
            snap.latency_p50 = book.get_metrics().latency_p50();
            snap.latency_p99 = book.get_metrics().latency_p99();

            book_to_gui.push(snap);
            last_snapshot_time = now;
        }
    }
}

int main() {
    Generator gen(100000.0, 50, 1, 100, 100.0, 2.5); //Orders/sec, num clients, min quantity, max quantity, reference price, stddev
    OrderBook book(10000000);

    SPSC<Order> gen_to_book(1024);
    SPSC<Snapshot> book_to_gui(16);

    thread producer(generator_thread_func, ref(gen), ref(gen_to_book));
    thread backend(backend_thread_func, ref(gen_to_book), ref(book_to_gui), ref(book));
    thread gui(gui_thread_func, ref(book_to_gui), ref(running));

    gui.join();
    running = false;

    producer.join();
    backend.join();

    return 0;
}