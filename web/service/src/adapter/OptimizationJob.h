#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <functional>
#include <mutex>
#include <unordered_map>

// Forward declarations
namespace dH {
    class Plan;
    class PlanOptimizer;
}
class DynamicCovarianceOptimizer;

namespace brimstone {

// Progress update sent to client
struct OptimizationProgress {
    int level;              // Pyramid level (0-3)
    int iteration;          // Current iteration
    double cost;            // Current cost value
    double freeEnergy;      // KL divergence (if computed)
    double entropy;         // Entropy term
    bool levelComplete;     // True when a level finishes
};

// Final result
struct OptimizationResult {
    bool success;
    double finalCost;
    int totalIterations;
    std::string errorMessage;
};

// Callback for progress updates
using ProgressCallback = std::function<void(const OptimizationProgress&)>;
using CompletionCallback = std::function<void(const OptimizationResult&)>;

// Job status
enum class JobStatus {
    Pending,
    Running,
    Completed,
    Cancelled,
    Failed
};

// Async optimization job wrapper
class OptimizationJob {
public:
    OptimizationJob(const std::string& plan_id, dH::Plan* plan);
    ~OptimizationJob();

    // Job control
    void start(ProgressCallback on_progress, CompletionCallback on_complete);
    void stop();

    // Status
    JobStatus get_status() const { return m_status.load(); }
    const std::string& get_plan_id() const { return m_plan_id; }
    int get_current_level() const { return m_current_level.load(); }
    int get_current_iteration() const { return m_current_iteration.load(); }

private:
    std::string m_plan_id;
    dH::Plan* m_plan;
    std::unique_ptr<dH::PlanOptimizer> m_optimizer;

    std::atomic<JobStatus> m_status{JobStatus::Pending};
    std::atomic<bool> m_stop_requested{false};
    std::atomic<int> m_current_level{0};
    std::atomic<int> m_current_iteration{0};

    std::unique_ptr<std::thread> m_thread;

    ProgressCallback m_on_progress;
    CompletionCallback m_on_complete;

    void run_optimization();

    // Static callback bridge
    static int OnIteration(DynamicCovarianceOptimizer* pOpt, void* pParam);
};

// Job manager - tracks all running jobs
class JobManager {
public:
    static JobManager& instance();

    // Job management
    std::string start_job(const std::string& plan_id, dH::Plan* plan,
                          ProgressCallback on_progress, CompletionCallback on_complete);
    bool stop_job(const std::string& job_id);
    OptimizationJob* get_job(const std::string& job_id);

    // Query jobs by plan
    std::string get_active_job_for_plan(const std::string& plan_id);

    // Cleanup completed jobs
    void cleanup_completed();

private:
    JobManager() = default;
    ~JobManager() = default;
    JobManager(const JobManager&) = delete;
    JobManager& operator=(const JobManager&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::unique_ptr<OptimizationJob>> m_jobs;
    int m_next_job_id = 1;
};

} // namespace brimstone
