#include "Routes.h"
#include "WebSocketHub.h"
#include "adapter/PlanAdapter.h"
#include "adapter/VolumeAdapter.h"
#include "adapter/OptimizationJob.h"
#include "adapter/ProgressCallback.h"

#include <Plan.h>
#include <Series.h>
#include <Beam.h>
#include <Structure.h>
#include <Histogram.h>

#include <nlohmann/json.hpp>
#include <iostream>

namespace brimstone {

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Utility helpers
// ---------------------------------------------------------------------------

crow::response Routes::json_response(const json& j, int status) {
    crow::response res(status, j.dump());
    res.add_header("Content-Type", "application/json");
    return res;
}

crow::response Routes::binary_response(const std::vector<uint8_t>& data,
                                        const std::string& content_type) {
    crow::response res(200);
    res.set_header("Content-Type", content_type);
    res.body.assign(reinterpret_cast<const char*>(data.data()), data.size());
    return res;
}

crow::response Routes::error_response(const std::string& message, int status) {
    json err = {{"error", message}};
    crow::response res(status, err.dump());
    res.add_header("Content-Type", "application/json");
    return res;
}

// ---------------------------------------------------------------------------
// Plan CRUD
// ---------------------------------------------------------------------------

crow::response Routes::list_plans(const crow::request& /*req*/) {
    try {
        auto ids = PlanStore::instance().get_plan_ids();
        json plans = json::array();
        for (const auto& id : ids) {
            auto* plan = PlanStore::instance().get_plan(id);
            if (plan) {
                plans.push_back(PlanAdapter::plan_summary_to_json(plan, id));
            }
        }
        return json_response(plans);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_plan(const crow::request& /*req*/, const std::string& id) {
    try {
        auto* plan = PlanStore::instance().get_plan(id);
        if (!plan) {
            return error_response("Plan not found", 404);
        }
        return json_response(PlanAdapter::plan_to_json(plan));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::create_plan(const crow::request& req) {
    try {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            return error_response("Invalid JSON body", 400);
        }

        std::string plan_id;
        if (body.contains("xml_path")) {
            plan_id = PlanStore::instance().load_plan_from_xml(body["xml_path"].get<std::string>());
        } else {
            std::string name = body.value("name", "Untitled");
            plan_id = PlanStore::instance().create_empty_plan(name);
        }

        json result = {{"id", plan_id}};
        return json_response(result, 201);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::delete_plan(const crow::request& /*req*/, const std::string& id) {
    try {
        if (!PlanStore::instance().remove_plan(id)) {
            return error_response("Plan not found", 404);
        }
        return json_response({{"deleted", true}});
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// CT / Dose slices
// ---------------------------------------------------------------------------

crow::response Routes::get_ct_slice(const crow::request& req,
                                     const std::string& plan_id, int slice) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        float wc = 40.0f, ww = 400.0f;
        if (req.url_params.get("window_center"))
            wc = static_cast<float>(std::stod(req.url_params.get("window_center")));
        if (req.url_params.get("window_width"))
            ww = static_cast<float>(std::stod(req.url_params.get("window_width")));

        auto data = VolumeAdapter::get_ct_slice(plan, slice, wc, ww);
        return binary_response(data);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_dose_slice(const crow::request& req,
                                       const std::string& plan_id, int slice) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        float max_dose = 0.0f;
        if (req.url_params.get("max_dose"))
            max_dose = static_cast<float>(std::stod(req.url_params.get("max_dose")));

        auto data = VolumeAdapter::get_dose_slice(plan, slice, max_dose);
        return binary_response(data);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_ct_info(const crow::request& /*req*/, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        return json_response(VolumeAdapter::get_ct_info(plan));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_dose_info(const crow::request& /*req*/, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        return json_response(VolumeAdapter::get_dose_info(plan));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// Volume data (3D)
// ---------------------------------------------------------------------------

crow::response Routes::get_ct_volume(const crow::request& req, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        int downsample = 1;
        if (req.url_params.get("downsample"))
            downsample = std::stoi(req.url_params.get("downsample"));

        auto data = VolumeAdapter::get_ct_volume(plan, downsample);
        return binary_response(data);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_dose_volume(const crow::request& req, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        int downsample = 1;
        float max_dose = 0.0f;
        if (req.url_params.get("downsample"))
            downsample = std::stoi(req.url_params.get("downsample"));
        if (req.url_params.get("max_dose"))
            max_dose = static_cast<float>(std::stod(req.url_params.get("max_dose")));

        auto data = VolumeAdapter::get_dose_volume(plan, downsample, max_dose);
        return binary_response(data);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// Structures
// ---------------------------------------------------------------------------

crow::response Routes::list_structures(const crow::request& /*req*/, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        auto* series = plan->GetSeries();
        if (!series) return error_response("Plan has no series", 500);

        json structures = json::array();
        int count = series->GetStructureCount();
        for (int i = 0; i < count; i++) {
            auto* structure = series->GetStructureAt(i);
            if (structure) {
                structures.push_back(PlanAdapter::structure_to_json(structure));
            }
        }
        return json_response(structures);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_structure(const crow::request& /*req*/,
                                      const std::string& plan_id, const std::string& name) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        auto* series = plan->GetSeries();
        if (!series) return error_response("Plan has no series", 500);

        auto* structure = series->GetStructureFromName(name);
        if (!structure) return error_response("Structure not found: " + name, 404);

        return json_response(PlanAdapter::structure_to_json(structure));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_structure_slice(const crow::request& /*req*/,
                                            const std::string& plan_id,
                                            const std::string& name, int slice) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        auto* series = plan->GetSeries();
        if (!series) return error_response("Plan has no series", 500);

        auto* structure = series->GetStructureFromName(name);
        if (!structure) return error_response("Structure not found: " + name, 404);

        auto data = VolumeAdapter::get_structure_slice(structure, slice);
        return binary_response(data);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// Beams
// ---------------------------------------------------------------------------

crow::response Routes::list_beams(const crow::request& /*req*/, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        json beams = json::array();
        int count = plan->GetBeamCount();
        for (int i = 0; i < count; i++) {
            auto* beam = plan->GetBeamAt(i);
            if (beam) {
                beams.push_back(PlanAdapter::beam_to_json(beam, i));
            }
        }
        return json_response(beams);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_beam(const crow::request& /*req*/,
                                 const std::string& plan_id, int index) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        if (index < 0 || index >= plan->GetBeamCount()) {
            return error_response("Beam index out of range", 404);
        }

        auto* beam = plan->GetBeamAt(index);
        return json_response(PlanAdapter::beam_to_json(beam, index));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::update_beam(const crow::request& req,
                                    const std::string& plan_id, int index) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        if (index < 0 || index >= plan->GetBeamCount()) {
            return error_response("Beam index out of range", 404);
        }

        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            return error_response("Invalid JSON body", 400);
        }

        auto* beam = plan->GetBeamAt(index);
        PlanAdapter::update_beam_from_json(beam, body);
        return json_response(PlanAdapter::beam_to_json(beam, index));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// DVH / Histograms
// ---------------------------------------------------------------------------

crow::response Routes::get_dvh(const crow::request& /*req*/,
                                const std::string& plan_id,
                                const std::string& structure_name) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        auto* series = plan->GetSeries();
        if (!series) return error_response("Plan has no series", 500);

        auto* structure = series->GetStructureFromName(structure_name);
        if (!structure) return error_response("Structure not found: " + structure_name, 404);

        auto* histogram = plan->GetHistogram(structure, false);
        if (!histogram) return error_response("No histogram available for structure", 404);

        return json_response(PlanAdapter::histogram_to_json(histogram, structure_name));
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_all_dvh(const crow::request& /*req*/, const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        auto* series = plan->GetSeries();
        if (!series) return error_response("Plan has no series", 500);

        json histograms = json::array();
        int count = series->GetStructureCount();
        for (int i = 0; i < count; i++) {
            auto* structure = series->GetStructureAt(i);
            if (!structure) continue;

            auto* histogram = plan->GetHistogram(structure, false);
            if (histogram) {
                histograms.push_back(
                    PlanAdapter::histogram_to_json(histogram, structure->GetName()));
            }
        }
        return json_response(histograms);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// Optimization
// ---------------------------------------------------------------------------

crow::response Routes::start_optimization(const crow::request& /*req*/,
                                           const std::string& plan_id) {
    try {
        auto* plan = PlanStore::instance().get_plan(plan_id);
        if (!plan) return error_response("Plan not found", 404);

        // Check for existing running job
        auto existing = JobManager::instance().get_active_job_for_plan(plan_id);
        if (!existing.empty()) {
            return error_response("Optimization already running for this plan", 409);
        }

        // Progress callback: broadcast via WebSocket
        auto on_progress = [plan_id](const OptimizationProgress& progress) {
            auto msg = ProgressSerializer::progress_to_json(progress).dump();
            WebSocketHub::instance().broadcast(plan_id, msg);
        };

        // Completion callback: broadcast final result
        auto on_complete = [plan_id](const OptimizationResult& result) {
            auto msg = ProgressSerializer::result_to_json(result).dump();
            WebSocketHub::instance().broadcast(plan_id, msg);
        };

        auto job_id = JobManager::instance().start_job(plan_id, plan, on_progress, on_complete);

        json result = {{"job_id", job_id}, {"plan_id", plan_id}, {"status", "running"}};
        return json_response(result, 202);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::stop_optimization(const crow::request& /*req*/,
                                          const std::string& plan_id) {
    try {
        auto job_id = JobManager::instance().get_active_job_for_plan(plan_id);
        if (job_id.empty()) {
            return error_response("No active optimization for this plan", 404);
        }

        JobManager::instance().stop_job(job_id);
        return json_response({{"stopped", true}, {"job_id", job_id}});
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

crow::response Routes::get_optimization_status(const crow::request& /*req*/,
                                                const std::string& plan_id) {
    try {
        auto job_id = JobManager::instance().get_active_job_for_plan(plan_id);
        if (job_id.empty()) {
            return json_response({{"status", "none"}, {"plan_id", plan_id}});
        }

        auto* job = JobManager::instance().get_job(job_id);
        if (!job) {
            return json_response({{"status", "none"}, {"plan_id", plan_id}});
        }

        auto status_json = ProgressSerializer::job_status_to_json(job);
        status_json["job_id"] = job_id;
        return json_response(status_json);
    } catch (const std::exception& e) {
        return error_response(e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// WebSocket
// ---------------------------------------------------------------------------

void Routes::register_websocket(crow::SimpleApp& app) {
    CROW_WEBSOCKET_ROUTE(app, "/ws/optimization/<str>")
        .onopen([](crow::websocket::connection& conn, const std::string& plan_id) {
            std::cout << "[WS] Client connected for plan: " << plan_id << std::endl;
            WebSocketHub::instance().add_connection(plan_id, &conn);
        })
        .onclose([](crow::websocket::connection& conn, const std::string& /*reason*/,
                     const std::string& plan_id) {
            std::cout << "[WS] Client disconnected for plan: " << plan_id << std::endl;
            WebSocketHub::instance().remove_connection(&conn);
        })
        .onmessage([](crow::websocket::connection& /*conn*/, const std::string& data,
                       bool /*is_binary*/) {
            // Client messages not used currently; could add command handling
            std::cout << "[WS] Received: " << data << std::endl;
        });
}

// ---------------------------------------------------------------------------
// Route registration
// ---------------------------------------------------------------------------

void Routes::register_routes(crow::SimpleApp& app) {
    // Plan CRUD
    CROW_ROUTE(app, "/api/plans").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req) { return list_plans(req); });

    CROW_ROUTE(app, "/api/plans").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) { return create_plan(req); });

    CROW_ROUTE(app, "/api/plans/<str>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return get_plan(req, id); });

    CROW_ROUTE(app, "/api/plans/<str>").methods(crow::HTTPMethod::DELETE)
        ([](const crow::request& req, const std::string& id) { return delete_plan(req, id); });

    // CT / Dose slices
    CROW_ROUTE(app, "/api/plans/<str>/ct/slices/<int>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id, int slice) {
            return get_ct_slice(req, id, slice);
        });

    CROW_ROUTE(app, "/api/plans/<str>/dose/slices/<int>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id, int slice) {
            return get_dose_slice(req, id, slice);
        });

    CROW_ROUTE(app, "/api/plans/<str>/ct/info").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return get_ct_info(req, id); });

    CROW_ROUTE(app, "/api/plans/<str>/dose/info").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return get_dose_info(req, id); });

    // Volume data
    CROW_ROUTE(app, "/api/plans/<str>/ct/volume").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return get_ct_volume(req, id); });

    CROW_ROUTE(app, "/api/plans/<str>/dose/volume").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return get_dose_volume(req, id); });

    // Structures
    CROW_ROUTE(app, "/api/plans/<str>/structures").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) {
            return list_structures(req, id);
        });

    CROW_ROUTE(app, "/api/plans/<str>/structures/<str>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id, const std::string& name) {
            return get_structure(req, id, name);
        });

    CROW_ROUTE(app, "/api/plans/<str>/structures/<str>/slices/<int>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id, const std::string& name, int slice) {
            return get_structure_slice(req, id, name, slice);
        });

    // Beams
    CROW_ROUTE(app, "/api/plans/<str>/beams").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return list_beams(req, id); });

    CROW_ROUTE(app, "/api/plans/<str>/beams/<int>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id, int index) {
            return get_beam(req, id, index);
        });

    CROW_ROUTE(app, "/api/plans/<str>/beams/<int>").methods(crow::HTTPMethod::PUT)
        ([](const crow::request& req, const std::string& id, int index) {
            return update_beam(req, id, index);
        });

    // DVH
    CROW_ROUTE(app, "/api/plans/<str>/dvh/<str>").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id, const std::string& name) {
            return get_dvh(req, id, name);
        });

    CROW_ROUTE(app, "/api/plans/<str>/dvh").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) { return get_all_dvh(req, id); });

    // Optimization
    CROW_ROUTE(app, "/api/plans/<str>/optimize").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, const std::string& id) {
            return start_optimization(req, id);
        });

    CROW_ROUTE(app, "/api/plans/<str>/optimize/stop").methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, const std::string& id) {
            return stop_optimization(req, id);
        });

    CROW_ROUTE(app, "/api/plans/<str>/optimize/status").methods(crow::HTTPMethod::GET)
        ([](const crow::request& req, const std::string& id) {
            return get_optimization_status(req, id);
        });

    // WebSocket
    register_websocket(app);

    std::cout << "[Routes] Registered 21 REST endpoints + WebSocket" << std::endl;
}

} // namespace brimstone
