#include "orderbook.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

void Orderbook::updateFromJson(const std::string& jsonStr) {
    std::lock_guard<std::mutex> lock(mtx);
    
    json j = json::parse(jsonStr);

    // Clear old data
    bids.clear();
    asks.clear();

    // Parse bids
    for (const auto& bid : j["bids"]) {
        double price = std::stod(bid[0].get<std::string>());
        double qty   = std::stod(bid[1].get<std::string>());
        if (qty > 0) bids[price] = qty;
    }

    // Parse asks
    for (const auto& ask : j["asks"]) {
        double price = std::stod(ask[0].get<std::string>());
        double qty   = std::stod(ask[1].get<std::string>());
        if (qty > 0) asks[price] = qty;
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

std::vector<std::pair<double, double>> Orderbook::getBidLevels(size_t depth) const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<double, double>> levels;
    for (auto it = bids.begin(); it != bids.end() && levels.size() < depth; ++it) {
        levels.emplace_back(it->first, it->second);
    }
    return levels;
}

std::vector<std::pair<double, double>> Orderbook::getAskLevels(size_t depth) const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<double, double>> levels;
    for (auto it = asks.begin(); it != asks.end() && levels.size() < depth; ++it) {
        levels.emplace_back(it->first, it->second);
    }
    return levels;
}

std::chrono::steady_clock::time_point Orderbook::getLastUpdateTime() {
    std::lock_guard<std::mutex> lock(mtx);
    return lastUpdateTime;
}

double Orderbook::simulateMarketBuy(double usdAmount) {
    std::lock_guard<std::mutex> lock(mtx);
    double remaining = usdAmount;
    double totalQty = 0.0;

    for (const auto& [price, qty] : asks) {
        double levelCost = price * qty;
        if (levelCost <= remaining) {
            totalQty += qty;
            remaining -= levelCost;
        } else {
            totalQty += remaining / price;
            break;
        }
    }

    return totalQty; // total asset units bought
}

double Orderbook::simulateMarketSell(double usdAmount) {
    std::lock_guard<std::mutex> lock(mtx);
    double remaining = usdAmount;
    double totalQty = 0.0;

    for (const auto& [price, qty] : bids) {
        double levelValue = price * qty;
        if (levelValue <= remaining) {
            totalQty += qty;
            remaining -= levelValue;
        } else {
            totalQty += remaining / price;
            break;
        }
    }

    return totalQty; // total asset units sold
}
