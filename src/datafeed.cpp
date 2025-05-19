#include "datafeed.h"
#include <chrono>

// TEMP: dummy data for now — replace with live websocket data
OrderBookSnapshot getCurrentOrderBookSnapshot() {
    OrderBookSnapshot snapshot;

    snapshot.bids = {
        {60000.0, 0.5},
        {59950.0, 1.0},
        {59900.0, 2.0}
    };

    snapshot.asks = {
        {60050.0, 0.3},
        {60100.0, 1.5},
        {60150.0, 2.5}
    };

    snapshot.timestamp = std::chrono::steady_clock::now();
    return snapshot;
}
