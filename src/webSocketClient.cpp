#include "webSocketClient.h"
#include <ixwebsocket/IXNetSystem.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <tradesim.h>

using json = nlohmann::json;

WebSocketClient::WebSocketClient(const std::string& url, Orderbook& ob)
    : endpointUrl(url), orderbook(ob), running(false) {
    ix::initNetSystem();
}

WebSocketClient::~WebSocketClient() {
    stop();
    ix::uninitNetSystem();
}

void WebSocketClient::start() {
    if (running) return;
    running = true;
    workerThread = std::thread(&WebSocketClient::runLoop, this);
}

void WebSocketClient::stop() {
    if (!running) return;
    running = false;
    {
        std::lock_guard<std::mutex> lock(mtx);
        cv.notify_all();
    }
    webSocket_.stop();
    if (workerThread.joinable())
        workerThread.join();
}

void WebSocketClient::runLoop() {
    while (running) {
        std::cout << "[WebSocketClient] Connecting to: " << endpointUrl << "\n";

        webSocket_.setUrl(endpointUrl);

        webSocket_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                handleMessage(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "[WebSocketClient] Connected.\n";
                subscribeToOrderbook();
            } else if (msg->type == ix::WebSocketMessageType::Close) {
                std::cout << "[WebSocketClient] Connection closed. Attempting to reconnect...\n";
            } else if (msg->type == ix::WebSocketMessageType::Error) {
                std::cerr << "[WebSocketClient] Error: " << msg->errorInfo.reason << std::endl;
            }
        });

        webSocket_.start();

        // Wait until it stops or fails
        while (webSocket_.getReadyState() == ix::ReadyState::Open && running) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::seconds(1));
        }

        if (!running) break;

        std::cout << "[WebSocketClient] Reconnecting in 2 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void WebSocketClient::subscribeToOrderbook() {
    json subscribeMsg = {
        {"op", "subscribe"},
        {"args", {{{"channel", "books"}, {"instId", "BTC-USDT"}}}}
    };
    webSocket_.send(subscribeMsg.dump());
}

void WebSocketClient::handleMessage(const std::string& message) {
    try {
        auto j = json::parse(message);

        if (j.contains("arg") && j["arg"]["channel"] == "books" && j.contains("data")) {
            const auto& data = j["data"][0];

            OrderBookSnapshot snapshot;
            snapshot.timestamp = std::chrono::high_resolution_clock::now();

            for (const auto& ask : data["asks"]) {
                double price = std::stod(ask[0].get<std::string>());
                double size = std::stod(ask[1].get<std::string>());
                snapshot.asks.emplace_back(price, size);
            }

            for (const auto& bid : data["bids"]) {
                double price = std::stod(bid[0].get<std::string>());
                double size = std::stod(bid[1].get<std::string>());
                snapshot.bids.emplace_back(price, size);
            }

            orderbook.update(snapshot);
        }
    } catch (const std::exception& e) {
        std::cerr << "[WebSocketClient] Failed to parse message: " << e.what() << std::endl;
    }
}
