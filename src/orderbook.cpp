#include "Orderbook.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

void Orderbook::updateFromJson(const std::string& jsonString) {
    std::lock_guard<std::mutex> lock(mtx);

    try {
        auto j = json::parse(jsonString);

        // This assumes OKX L2 message: { "arg": {...}, "data": [ { "bids": [...], "asks": [...] } ] }
        if (!j.contains("data") || !j["data"].is_array() || j["data"].empty()) {
            std::cerr << "[Orderbook] Malformed update JSON: no data array\n";
            return;
        }

        const auto& book = j["data"][0];

        if (book.contains("bids")) {
            bids.clear();
            for (const auto& level : book["bids"]) {
                if (level.size() < 2) continue;
                double price = std::stod(level[0].get<std::string>());
                double quantity = std::stod(level[1].get<std::string>());
                if (quantity > 0.0) bids[price] = quantity;
            }
        }

        if (book.contains("asks")) {
            asks.clear();
            for (const auto& level : book["asks"]) {
                if (level.size() < 2) continue;
                double price = std::stod(level[0].get<std::string>());
                double quantity = std::stod(level[1].get<std::string>());
                if (quantity > 0.0) asks[price] = quantity;
            }
        }

        lastUpdateTime = std::chrono::steady_clock::now();
    } catch (const std::exception& e) {
        std::cerr << "[Orderbook] Failed to parse/update: " << e.what() << '\n';
    }
}

double Orderbook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mtx);
    return bids.empty() ? 0.0 : bids.begin()->first;
}

double Orderbook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mtx);
    return asks.empty() ? 0.0 : asks.begin()->first;
}

double Orderbook::simulateMarketBuy(double usdAmount) {
    std::lock_guard<std::mutex> lock(mtx);
    double remaining = usdAmount;
    double cost = 0.0;

    for (auto it = asks.begin(); it != asks.end() && remaining > 0; ++it) {
        double price = it->first;
        double qty = it->second;
        double value = price * qty;

        if (value <= remaining) {
            cost += value;
            remaining -= value;
        } else {
            double partialQty = remaining / price;
            cost += partialQty * price;
            remaining = 0;
        }
    }

    return remaining > 0 ? 0.0 : cost / (usdAmount / getBestAsk());  // normalized average
}

double Orderbook::simulateMarketSell(double usdAmount) {
    std::lock_guard<std::mutex> lock(mtx);
    double remaining = usdAmount;
    double cost = 0.0;

    for (auto it = bids.begin(); it != bids.end() && remaining > 0; ++it) {
        double price = it->first;
        double qty = it->second;
        double value = price * qty;

        if (value <= remaining) {
            cost += value;
            remaining -= value;
        } else {
            double partialQty = remaining / price;
            cost += partialQty * price;
            remaining = 0;
        }
    }

    return remaining > 0 ? 0.0 : cost / (usdAmount / getBestBid());
}

std::vector<std::pair<double, double>> Orderbook::getBidLevels(size_t depth) const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<double, double>> levels;
    for (const auto& [price, qty] : bids) {
        levels.emplace_back(price, qty);
        if (levels.size() >= depth) break;
    }
    return levels;
}

std::vector<std::pair<double, double>> Orderbook::getAskLevels(size_t depth) const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<double, double>> levels;
    for (const auto& [price, qty] : asks) {
        levels.emplace_back(price, qty);
        if (levels.size() >= depth) break;
    }
    return levels;
}

std::chrono::steady_clock::time_point Orderbook::getLastUpdateTime() const {
    std::lock_guard<std::mutex> lock(mtx);
    return lastUpdateTime;
}
