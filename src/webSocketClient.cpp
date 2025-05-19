#include "webSocketClient.h"
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebSocketClient::WebSocketClient(const std::string& url, Orderbook& ob)
    : endpointUrl(url), orderbook(ob) {}

void WebSocketClient::start() {
    ix::WebSocket ws;
    ws.setUrl(endpointUrl);

    ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            orderbook.updateFromJson(msg->str);
            std::cout << "[Update] Bid: " << orderbook.getBestBid()
                      << ", Ask: " << orderbook.getBestAsk() << std::endl;
        }
    });

    ws.start();

    std::cout << "[WebSocket] Connected to " << endpointUrl << std::endl;
    while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
}

void WebSocketClient::stop() {
    running = false;
}

void WebSocketClient::connect() {
    webSocket_.setUrl(endpointUrl);

    webSocket_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            orderbook.updateFromJson(msg->str);
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "[WebSocket] Connected to " << endpointUrl << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cerr << "[WebSocket] Error: " << msg->errorInfo.reason << std::endl;
        }
    });

    webSocket_.start();
}
