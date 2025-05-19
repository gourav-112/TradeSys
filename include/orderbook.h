#pragma once

#include <map>
#include <string>
#include <mutex>
#include <vector>
#include <utility>
#include <chrono>

class Orderbook {
private:
    // Price -> Quantity
    std::map<double, double, std::greater<>> bids;  // highest first
    std::map<double, double> asks;                  // lowest first

    mutable std::mutex mtx; // For thread safety
    std::chrono::steady_clock::time_point lastUpdateTime;

public:
    Orderbook() = default;

    // Update entire snapshot from JSON string (L2 level)
    void updateFromJson(const std::string& jsonString);

    // Return best bid and best ask
    double getBestBid() const;
    double getBestAsk() const;

    double simulateMarketBuy(double usdAmount);   // consumes from asks
    double simulateMarketSell(double usdAmount);  // consumes from bids


    // Optional: get full depth (e.g., for slippage modeling)
    std::vector<std::pair<double, double>> getBidLevels(size_t depth = 10) const;
    std::vector<std::pair<double, double>> getAskLevels(size_t depth = 10) const;
    std::chrono::steady_clock::time_point getLastUpdateTime(); 
};
