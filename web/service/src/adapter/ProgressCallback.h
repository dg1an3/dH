#pragma once

#include "OptimizationJob.h"
#include <nlohmann/json.hpp>
#include <string>

namespace brimstone {

using json = nlohmann::json;

// Convert optimization progress/result to JSON for WebSocket transmission
class ProgressSerializer {
public:
    static json progress_to_json(const OptimizationProgress& progress);
    static json result_to_json(const OptimizationResult& result);
    static json job_status_to_json(OptimizationJob* job);

    static std::string status_to_string(JobStatus status);
};

} // namespace brimstone
