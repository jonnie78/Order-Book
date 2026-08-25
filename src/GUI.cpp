#include"GUI.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

using namespace std;

//Function to run on GUI thread
void gui_thread_func(SPSC<Snapshot>& book_to_gui, atomic<bool>& running) {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Order Book", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    Snapshot current_snapshot;

    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwWindowShouldClose(window)) {
        running = false;
        }

        // Drain buffer
        Snapshot temp;
        while (book_to_gui.pop(temp)) {
            current_snapshot = temp;
        }
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        float w = (float)display_w;
        float h = (float)display_h;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_depth_chart(current_snapshot, w, h);
        render_recent_orders(current_snapshot, w ,h);
        render_metrics(current_snapshot, w, h);

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
//Function to render depth charts
void render_depth_chart(const Snapshot& snap, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, h * 0.8f), ImGuiCond_Always);
    ImGui::Begin("Depth Chart");
    vector<double> bid_prices, bid_vols, ask_prices, ask_vols;
    for (auto& pv : snap.bid_levels) { bid_prices.push_back(pv.price); bid_vols.push_back(pv.volume); }
    for (auto& pv : snap.ask_levels) { ask_prices.push_back(pv.price); ask_vols.push_back(pv.volume); }

    ImVec2 plot_size = ImVec2(-1, h * 0.8f - 60);
    if (ImPlot::BeginPlot("Order Book Depth", plot_size)) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlotSpec bid_spec;
        bid_spec.FillColor = ImVec4(0.0f, 0.8f, 0.0f, 0.6f); //Green
        ImPlot::PlotBars("Bids", bid_prices.data(), bid_vols.data(), (int)bid_prices.size(), 1.0, bid_spec);
        ImPlotSpec ask_spec;
        ask_spec.FillColor = ImVec4(0.8f, 0.0f, 0.0f, 0.6f); // red
        ImPlot::PlotBars("Asks", ask_prices.data(), ask_vols.data(), (int)ask_prices.size(), 1.0, ask_spec);
        ImPlot::EndPlot();
    }

    if (!snap.bid_levels.empty() && !snap.ask_levels.empty()) {
        uint32_t best_bid = snap.bid_levels.front().price;
        uint32_t best_ask = snap.ask_levels.front().price;
        double spread = static_cast<double>(best_ask) - static_cast<double>(best_bid);
        ImGui::Text("Spread: %.2f", spread);
    }
    ImGui::End();
}
//Function to render recent orders
void render_recent_orders(const Snapshot& snap, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(0, h * 0.8f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w * 0.5f, h * 0.2f), ImGuiCond_Always);
    ImGui::Begin("Recent Orders");
    ImGui::SetWindowFontScale(1.4f);
    if (ImGui::BeginTable("orders", 4)) {
        ImGui::TableSetupColumn("ID"); ImGui::TableSetupColumn("Side");
        ImGui::TableSetupColumn("Price"); ImGui::TableSetupColumn("Qty");
        ImGui::TableHeadersRow();
        for (int i = 0; i < snap.recent_count; ++i) {
            const Order& o = snap.recent_orders[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%llu", o.order_id);
            ImGui::TableSetColumnIndex(1); ImGui::Text(o.is_buy ? "BUY" : "SELL");
            ImGui::TableSetColumnIndex(2); ImGui::Text("%u", o.price);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%u", o.quantity);
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::EndTable();
    }
    ImGui::End();
}
//Function to render metrics
void render_metrics(const Snapshot& snap, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.8f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w * 0.5f, h * 0.2f), ImGuiCond_Always);
    ImGui::Begin("Metrics");
    ImGui::SetWindowFontScale(1.4f);
    ImGui::Text("Throughput: %.1f orders/sec", snap.throughput);
    ImGui::Text("Total Volume: %llu", snap.total_volume);
    ImGui::Text("Latency p50: %.0f ns", snap.latency_p50);
    ImGui::Text("Latency p99: %.0f ns", snap.latency_p99);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}