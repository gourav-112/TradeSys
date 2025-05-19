#ifndef TRADE_CONFIG_H
#define TRADE_CONFIG_H

#include <string>

struct TradeConfig {
    std::string exchange = "OKX";
    std::string symbol = "BTC-USDT-SWAP";
    std::string orderType = "market";  // Only market supported for now
    double quantityUSD = 100.0;        // Amount to simulate
    double volatility = 0.0;           // Placeholder; will be used later
    double feeTier = 0.001;            // Exchange-specific fee (taker for now)
};

#endif
