#include "ProgressCallback.h"

namespace brimstone {

json ProgressSerializer::progress_to_json(const OptimizationProgress& progress) {
    json j;

    if (progress.levelComplete) {
        j["type"] = "level_complete";
    } else {
        j["type"] = "iteration";
    }

    j["level"] = progress.level;
    j["iteration"] = progress.iteration;
    j["cost"] = progress.cost;
    j["freeEnergy"] = progress.freeEnergy;
    j["entropy"] = progress.entropy;

    return j;
}

json ProgressSerializer::result_to_json(const OptimizationResult& result) {
    json j;
    j["type"] = "complete";
    j["success"] = result.success;
    j["finalCost"] = result.finalCost;
    j["totalIterations"] = result.totalIterations;

    if (!result.errorMessage.empty()) {
        j["error"] = result.errorMessage;
    }

    return j;
}

json ProgressSerializer::job_status_to_json(OptimizationJob* job) {
    json j;

    if (!job) {
        j["error"] = "job not found";
        return j;
    }

    j["status"] = status_to_string(job->get_status());
    j["planId"] = job->get_plan_id();
    j["currentLevel"] = job->get_current_level();
    j["currentIteration"] = job->get_current_iteration();

    return j;
}

std::string ProgressSerializer::status_to_string(JobStatus status) {
    switch (status) {
        case JobStatus::Pending: return "pending";
        case JobStatus::Running: return "running";
        case JobStatus::Completed: return "completed";
        case JobStatus::Cancelled: return "cancelled";
        case JobStatus::Failed: return "failed";
        default: return "unknown";
    }
}

} // namespace brimstone
