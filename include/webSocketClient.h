#pragma once

#include <string>
#include <functional>
#include <memory>
#include "orderbook.h"
#include <ixwebsocket/IXWebSocket.h>

struct Order {
    double price;
    double quantity;
};

struct OrderBookSnapshot {
    std::vector<Order> bids;
    std::vector<Order> asks;
};

class WebSocketClient {
public:
    WebSocketClient(const std::string& url, Orderbook& ob);
    void start();
    void stop();
    void connect();

private:
    std::string endpointUrl;
    Orderbook& orderbook;
    ix::WebSocket webSocket_;
    bool running = false;
};
