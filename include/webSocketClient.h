#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include <atomic>
#include <thread>
#include "orderbook.h"

class WebSocketClient {
public:
    WebSocketClient(const std::string& url, Orderbook& ob);
    ~WebSocketClient();

    void start();
    void stop();

private:
    void connect();
    void subscribeToOrderbook();

    std::string endpointUrl;
    Orderbook& orderbook;
    ix::WebSocket webSocket_;
    std::atomic<bool> running;

    void runLoop();  // <- This is the reconnection loop
    void subscribeToOrderbook();
    void handleMessage(const std::string& message);

    std::thread workerThread;
    std::mutex mtx;
    std::condition_variable cv;
};
