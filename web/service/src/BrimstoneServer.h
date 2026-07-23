#pragma once

#include <crow.h>
#include <string>
#include <atomic>

namespace brimstone {

class BrimstoneServer {
public:
    BrimstoneServer() = default;
    ~BrimstoneServer();

    // Configure the server (port, threads, middleware)
    void configure(uint16_t port = 8080, int threads = 4);

    // Start the server (blocking)
    void start();

    // Stop the server gracefully
    void stop();

private:
    crow::SimpleApp m_app;
    uint16_t m_port = 8080;
    std::atomic<bool> m_running{false};
};

} // namespace brimstone
