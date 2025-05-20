#pragma once

#include <map>
#include <string>
#include <mutex>
#include <vector>
#include <utility>
#include <chrono>

class Orderbook {
public:
    Orderbook() = default;

    // Updates the entire snapshot from a raw JSON string (OKX L2 depth format)
    void updateFromJson(const std::string& jsonString);

    // Returns best bid (highest buy price)
    double getBestBid() const;

    // Returns best ask (lowest sell price)
    double getBestAsk() const;

    // Simulates a market buy (consumes from asks)
    double simulateMarketBuy(double usdAmount);

    // Simulates a market sell (consumes from bids)
    double simulateMarketSell(double usdAmount);

    // Optional: get top N price levels
    std::vector<std::pair<double, double>> getBidLevels(size_t depth = 10) const;
    std::vector<std::pair<double, double>> getAskLevels(size_t depth = 10) const;

    // Returns the time of last update
    std::chrono::steady_clock::time_point getLastUpdateTime() const;

private:
    std::map<double, double, std::greater<>> bids;  // price -> quantity
    std::map<double, double> asks;
    mutable std::mutex mtx;
    std::chrono::steady_clock::time_point lastUpdateTime;
};
