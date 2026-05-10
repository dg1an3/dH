#pragma once

#include <crow.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <functional>

namespace brimstone {

// WebSocket connection manager
// Manages connections per plan for targeted broadcasts
class WebSocketHub {
public:
    static WebSocketHub& instance();

    // Connection management
    void add_connection(const std::string& plan_id, crow::websocket::connection* conn);
    void remove_connection(crow::websocket::connection* conn);

    // Broadcasting
    void broadcast(const std::string& plan_id, const std::string& message);
    void broadcast_all(const std::string& message);
    void send_to(crow::websocket::connection* conn, const std::string& message);

    // Query
    size_t get_connection_count(const std::string& plan_id) const;
    size_t get_total_connections() const;

private:
    WebSocketHub() = default;
    ~WebSocketHub() = default;
    WebSocketHub(const WebSocketHub&) = delete;
    WebSocketHub& operator=(const WebSocketHub&) = delete;

    mutable std::mutex m_mutex;

    // plan_id -> set of connections
    std::unordered_map<std::string, std::unordered_set<crow::websocket::connection*>> m_plan_connections;

    // connection -> plan_id (reverse lookup for removal)
    std::unordered_map<crow::websocket::connection*, std::string> m_connection_plan;
};

} // namespace brimstone
