#pragma once

#include <crow.h>

namespace brimstone {

// REST API route handlers
class Routes {
public:
    // Register all routes with the Crow app
    static void register_routes(crow::SimpleApp& app);

private:
    // Plan CRUD
    static crow::response list_plans(const crow::request& req);
    static crow::response get_plan(const crow::request& req, const std::string& id);
    static crow::response create_plan(const crow::request& req);
    static crow::response delete_plan(const crow::request& req, const std::string& id);

    // CT/Dose slices
    static crow::response get_ct_slice(const crow::request& req,
                                       const std::string& plan_id, int slice);
    static crow::response get_dose_slice(const crow::request& req,
                                         const std::string& plan_id, int slice);
    static crow::response get_ct_info(const crow::request& req, const std::string& plan_id);
    static crow::response get_dose_info(const crow::request& req, const std::string& plan_id);

    // Volume data (for 3D rendering)
    static crow::response get_ct_volume(const crow::request& req, const std::string& plan_id);
    static crow::response get_dose_volume(const crow::request& req, const std::string& plan_id);

    // Structures
    static crow::response list_structures(const crow::request& req, const std::string& plan_id);
    static crow::response get_structure(const crow::request& req,
                                        const std::string& plan_id, const std::string& name);
    static crow::response get_structure_slice(const crow::request& req,
                                              const std::string& plan_id,
                                              const std::string& name, int slice);

    // Beams
    static crow::response list_beams(const crow::request& req, const std::string& plan_id);
    static crow::response get_beam(const crow::request& req,
                                   const std::string& plan_id, int index);
    static crow::response update_beam(const crow::request& req,
                                      const std::string& plan_id, int index);

    // DVH / Histograms
    static crow::response get_dvh(const crow::request& req,
                                  const std::string& plan_id, const std::string& structure_name);
    static crow::response get_all_dvh(const crow::request& req, const std::string& plan_id);

    // Optimization
    static crow::response start_optimization(const crow::request& req, const std::string& plan_id);
    static crow::response stop_optimization(const crow::request& req, const std::string& plan_id);
    static crow::response get_optimization_status(const crow::request& req,
                                                  const std::string& plan_id);

    // WebSocket handler registration
    static void register_websocket(crow::SimpleApp& app);

    // Utility
    static crow::response json_response(const nlohmann::json& j, int status = 200);
    static crow::response binary_response(const std::vector<uint8_t>& data,
                                          const std::string& content_type = "application/octet-stream");
    static crow::response error_response(const std::string& message, int status = 400);
};

} // namespace brimstone
