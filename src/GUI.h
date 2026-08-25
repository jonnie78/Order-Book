#pragma once
#include "Structs.h"
#include "Ring-Buffer.h"
#include <atomic>

//Function to run GUI thread to render all parts
void gui_thread_func(SPSC<Snapshot>& book_to_gui, std::atomic<bool>& running);
//Function to render depth chart
void render_depth_chart(const Snapshot& snap, float w, float h);
//Function to render recent orders
void render_recent_orders(const Snapshot& snap, float w, float h);
//Function to render metrics
void render_metrics(const Snapshot& snap, float w, float h);