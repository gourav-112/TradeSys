#pragma once
#include <vector>
#include <chrono>

struct OrderLevel {
    double price;
    double quantity;
};

using OrderBookSide = std::vector<OrderLevel>;

struct OrderBookSnapshot {
    OrderBookSide bids;
    OrderBookSide asks;
    std::chrono::steady_clock::time_point timestamp;
};

enum class Side { Buy, Sell };

struct TradeResult {
    double executedQuantity;
    double averagePrice;
    double totalCost;
    double totalProceeds;
    double slippage;
    double feesPaid;
    double marketImpact;
    double makerTakerRatio;
    double internalLatency;
};

TradeResult simulateMarketOrder(
    const OrderBookSnapshot& book,
    Side side,
    double quantity,
    int feeTier,
    std::chrono::steady_clock::time_point tradeTimestamp
);
