#include "BrimstoneServer.h"
#include "Routes.h"
#include <iostream>

namespace brimstone {

BrimstoneServer::~BrimstoneServer() {
    if (m_running) {
        stop();
    }
}

void BrimstoneServer::configure(uint16_t port, int threads) {
    m_port = port;

    // CORS: add Access-Control headers to every response
    m_app.after_handle([](crow::response& res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    });

    // Handle CORS preflight
    CROW_CATCHALL_ROUTE(m_app)([](const crow::request& req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
            crow::response res(204);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return res;
        }
        return crow::response(404);
    });

    // Register REST routes and WebSocket handlers
    Routes::register_routes(m_app);

    m_app.port(m_port).multithreaded().concurrency(threads);

    std::cout << "[BrimstoneServer] Configured on port " << m_port
              << " with " << threads << " threads" << std::endl;
}

void BrimstoneServer::start() {
    m_running = true;
    std::cout << "[BrimstoneServer] Starting on http://localhost:" << m_port << std::endl;
    m_app.run();
    m_running = false;
    std::cout << "[BrimstoneServer] Stopped" << std::endl;
}

void BrimstoneServer::stop() {
    std::cout << "[BrimstoneServer] Shutting down..." << std::endl;
    m_app.stop();
    m_running = false;
}

} // namespace brimstone
