#include <vector>
#include <cmath>
#include <chrono>

struct OrderLevel {
    double price;
    double quantity;
};

using OrderBookSide = std::vector<OrderLevel>;  // sorted descending for bids, ascending for asks

struct OrderBookSnapshot {
    OrderBookSide bids;
    OrderBookSide asks;
    std::chrono::steady_clock::time_point timestamp;
};

enum class Side { Buy, Sell };

struct TradeResult {
    double executedQuantity;    // How much was filled
    double averagePrice;        // Weighted avg execution price
    double totalCost;           // total cost including fees (for buys)
    double totalProceeds;       // total proceeds net of fees (for sells)
    double slippage;            // execution price vs mid price
    double feesPaid;            // total fees
    double marketImpact;        // liquidity consumed ratio
    double makerTakerRatio;     // 0 = all taker (for market orders)
    double latencyMicroseconds; // simulated or measured latency
};

// Simplified fee calculation (e.g. feeTier 1 = 0.1%)
double calculateFees(double tradeValue, int feeTier, bool isMaker) {
    double feeRate = 0.001 * feeTier; // example: tier 1 = 0.1%
    if (isMaker) feeRate /= 2.0;      // makers pay half fee
    return tradeValue * feeRate;
}

TradeResult simulateMarketOrder(
    const OrderBookSnapshot& book,
    Side side,
    double quantity,           // base asset quantity to buy or sell
    int feeTier,
    std::chrono::steady_clock::time_point tradeTimestamp
) {
    const OrderBookSide& levels = (side == Side::Buy) ? book.asks : book.bids;
    double remainingQty = quantity;
    double executedValue = 0.0; // sum of (price * quantity)
    double executedQty = 0.0;

    // Walk through order book levels to fill the order
    for (const auto& level : levels) {
        if (remainingQty <= 0) break;
        double fillQty = std::min(remainingQty, level.quantity);
        executedValue += fillQty * level.price;
        executedQty += fillQty;
        remainingQty -= fillQty;
    }

    // If not fully filled, executedQty < quantity

    double avgPrice = (executedQty > 0) ? (executedValue / executedQty) : 0.0;

    // Mid price calculation
    double bestBid = (!book.bids.empty()) ? book.bids.front().price : 0.0;
    double bestAsk = (!book.asks.empty()) ? book.asks.front().price : 0.0;
    double midPrice = (bestBid > 0 && bestAsk > 0) ? (bestBid + bestAsk) / 2.0 : 0.0;

    // Slippage: difference between average execution price and mid price
    double slippage = (side == Side::Buy) ? (avgPrice - midPrice) / midPrice : (midPrice - avgPrice) / midPrice;

    // Fees (all market orders are taker here)
    double fees = calculateFees(executedValue, feeTier, /*isMaker=*/false);

    // Total cost/proceeds including fees
    double totalCost = 0.0, totalProceeds = 0.0;
    if (side == Side::Buy) {
        totalCost = executedValue + fees;
    } else {
        totalProceeds = executedValue - fees;
    }

    // Market impact = quantity filled / total visible liquidity on that side
    double totalLiquidity = 0.0;
    for (const auto& lvl : levels)
        totalLiquidity += lvl.quantity;
    double marketImpact = (totalLiquidity > 0) ? (executedQty / totalLiquidity) : 0.0;

    // Latency simulation
    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(tradeTimestamp - book.timestamp).count();

    return TradeResult{
        executedQty,
        avgPrice,
        totalCost,
        totalProceeds,
        slippage,
        fees,
        marketImpact,
        0.0,           // makerTakerRatio: 0 for market order (all taker)
        static_cast<double>(latency)
    };
}
