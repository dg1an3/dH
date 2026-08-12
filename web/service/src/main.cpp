#include "BrimstoneServer.h"
#include "adapter/PlanAdapter.h"
#include <iostream>
#include <csignal>
#include <string>
#include <cstdlib>

static brimstone::BrimstoneServer* g_server = nullptr;

void signal_handler(int signum) {
    std::cout << "\n[main] Received signal " << signum << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

void print_usage(const char* program) {
    std::cerr << "Usage: " << program << " [options]" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  --port <port>       Server port (default: 8080)" << std::endl;
    std::cerr << "  --threads <count>   Worker threads (default: 4)" << std::endl;
    std::cerr << "  --load <xml_path>   Preload a plan from XML file" << std::endl;
    std::cerr << "  --help              Show this message" << std::endl;
}

int main(int argc, char* argv[]) {
    uint16_t port = 8080;
    int threads = 4;
    std::string preload_path;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<uint16_t>(std::atoi(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            threads = std::atoi(argv[++i]);
        } else if (arg == "--load" && i + 1 < argc) {
            preload_path = argv[++i];
        } else if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    // Preload plan if requested
    if (!preload_path.empty()) {
        std::cout << "[main] Loading plan from: " << preload_path << std::endl;
        try {
            auto id = brimstone::PlanStore::instance().load_plan_from_xml(preload_path);
            std::cout << "[main] Plan loaded with id: " << id << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[main] Failed to load plan: " << e.what() << std::endl;
            return 1;
        }
    }

    // Set up signal handling for graceful shutdown
    brimstone::BrimstoneServer server;
    g_server = &server;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    server.configure(port, threads);
    server.start();

    g_server = nullptr;
    return 0;
}
