#include "WebSocketHub.h"
#include <iostream>

namespace brimstone {

WebSocketHub& WebSocketHub::instance() {
    static WebSocketHub instance;
    return instance;
}

void WebSocketHub::add_connection(const std::string& plan_id, crow::websocket::connection* conn) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_plan_connections[plan_id].insert(conn);
    m_connection_plan[conn] = plan_id;

    std::cout << "[WebSocketHub] Connection added for plan " << plan_id
              << " (total: " << m_connection_plan.size() << ")" << std::endl;
}

void WebSocketHub::remove_connection(crow::websocket::connection* conn) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_connection_plan.find(conn);
    if (it != m_connection_plan.end()) {
        const std::string& plan_id = it->second;

        auto plan_it = m_plan_connections.find(plan_id);
        if (plan_it != m_plan_connections.end()) {
            plan_it->second.erase(conn);
            if (plan_it->second.empty()) {
                m_plan_connections.erase(plan_it);
            }
        }

        m_connection_plan.erase(it);

        std::cout << "[WebSocketHub] Connection removed for plan " << plan_id
                  << " (total: " << m_connection_plan.size() << ")" << std::endl;
    }
}

void WebSocketHub::broadcast(const std::string& plan_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_plan_connections.find(plan_id);
    if (it == m_plan_connections.end()) {
        return;
    }

    for (crow::websocket::connection* conn : it->second) {
        try {
            conn->send_text(message);
        } catch (const std::exception& e) {
            std::cerr << "[WebSocketHub] Send error: " << e.what() << std::endl;
        }
    }
}

void WebSocketHub::broadcast_all(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [conn, plan_id] : m_connection_plan) {
        try {
            conn->send_text(message);
        } catch (const std::exception& e) {
            std::cerr << "[WebSocketHub] Send error: " << e.what() << std::endl;
        }
    }
}

void WebSocketHub::send_to(crow::websocket::connection* conn, const std::string& message) {
    try {
        conn->send_text(message);
    } catch (const std::exception& e) {
        std::cerr << "[WebSocketHub] Send error: " << e.what() << std::endl;
    }
}

size_t WebSocketHub::get_connection_count(const std::string& plan_id) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_plan_connections.find(plan_id);
    if (it != m_plan_connections.end()) {
        return it->second.size();
    }
    return 0;
}

size_t WebSocketHub::get_total_connections() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connection_plan.size();
}

} // namespace brimstone
