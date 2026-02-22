#include "OptimizationJob.h"

#include <Plan.h>
#include <PlanOptimizer.h>
#include <ConjGradOptimizer.h>
#include <Prescription.h>
#include <PlanPyramid.h>
#include <VectorN.h>

#include <sstream>
#include <iomanip>

namespace brimstone {

// Context passed through the optimizer callback
struct CallbackContext {
    OptimizationJob* job;
    std::atomic<bool>* stop_requested;
    ProgressCallback* on_progress;
    int current_level;
};

OptimizationJob::OptimizationJob(const std::string& plan_id, dH::Plan* plan)
    : m_plan_id(plan_id)
    , m_plan(plan)
{
}

OptimizationJob::~OptimizationJob() {
    stop();
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
}

void OptimizationJob::start(ProgressCallback on_progress, CompletionCallback on_complete) {
    if (m_status.load() != JobStatus::Pending) {
        return;  // Already started
    }

    m_on_progress = std::move(on_progress);
    m_on_complete = std::move(on_complete);
    m_status.store(JobStatus::Running);

    m_thread = std::make_unique<std::thread>(&OptimizationJob::run_optimization, this);
}

void OptimizationJob::stop() {
    m_stop_requested.store(true);
}

int OptimizationJob::OnIteration(DynamicCovarianceOptimizer* pOpt, void* pParam) {
    auto* ctx = static_cast<CallbackContext*>(pParam);

    if (ctx->stop_requested->load()) {
        return FALSE;  // Stop optimization
    }

    // Build progress update
    OptimizationProgress progress;
    progress.level = ctx->current_level;
    progress.iteration = pOpt->get_num_iterations();
    progress.cost = pOpt->GetFinalValue();
    progress.freeEnergy = pOpt->GetFreeEnergy();
    progress.entropy = pOpt->GetEntropy();
    progress.levelComplete = false;

    // Update atomic counters
    ctx->job->m_current_level.store(progress.level);
    ctx->job->m_current_iteration.store(progress.iteration);

    // Fire callback
    if (*ctx->on_progress) {
        (*ctx->on_progress)(progress);
    }

    return TRUE;  // Continue optimization
}

void OptimizationJob::run_optimization() {
    OptimizationResult result;
    result.success = false;
    result.totalIterations = 0;

    try {
        // Create optimizer
        m_optimizer = std::make_unique<dH::PlanOptimizer>(m_plan);

        // Get initial state from plan
        CVectorN<> vState;
        m_optimizer->GetStateVectorFromPlan(vState);

        // Setup callback context
        CallbackContext ctx;
        ctx.job = this;
        ctx.stop_requested = &m_stop_requested;
        ctx.on_progress = &m_on_progress;
        ctx.current_level = dH::PlanPyramid::MAX_SCALES - 1;

        // Iterate through pyramid levels (coarse to fine)
        for (int level = dH::PlanPyramid::MAX_SCALES - 1; level >= 0; --level) {
            if (m_stop_requested.load()) {
                result.errorMessage = "Cancelled by user";
                m_status.store(JobStatus::Cancelled);
                break;
            }

            ctx.current_level = level;
            m_current_level.store(level);

            // Get optimizer for this level
            DynamicCovarianceOptimizer* pOpt = m_optimizer->GetOptimizer(level);
            if (!pOpt) {
                continue;
            }

            // Set callback
            pOpt->SetCallback(&OnIteration, &ctx);

            // Enable adaptive variance and free energy computation
            pOpt->SetAdaptiveVariance(true, 25.0, 50.0);
            pOpt->SetComputeFreeEnergy(true);

            // Run optimization at this level
            pOpt->minimize(vState);

            result.totalIterations += pOpt->get_num_iterations();

            // Send level complete notification
            OptimizationProgress levelComplete;
            levelComplete.level = level;
            levelComplete.iteration = pOpt->get_num_iterations();
            levelComplete.cost = pOpt->GetFinalValue();
            levelComplete.freeEnergy = pOpt->GetFreeEnergy();
            levelComplete.entropy = pOpt->GetEntropy();
            levelComplete.levelComplete = true;

            if (m_on_progress) {
                m_on_progress(levelComplete);
            }

            // Filter state vector to next finer level
            if (level > 0) {
                CVectorN<> vStateNext;
                m_optimizer->InvFilterStateVector(level, vState, vStateNext);
                vState = vStateNext;
            }
        }

        if (!m_stop_requested.load()) {
            // Apply final state to plan
            m_optimizer->SetStateVectorToPlan(vState);

            // Update histograms
            m_plan->UpdateAllHisto();

            result.success = true;
            result.finalCost = m_optimizer->GetOptimizer(0)->GetFinalValue();
            m_status.store(JobStatus::Completed);
        }

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
        m_status.store(JobStatus::Failed);
    }

    // Fire completion callback
    if (m_on_complete) {
        m_on_complete(result);
    }
}

// JobManager implementation
JobManager& JobManager::instance() {
    static JobManager instance;
    return instance;
}

std::string JobManager::start_job(
    const std::string& plan_id,
    dH::Plan* plan,
    ProgressCallback on_progress,
    CompletionCallback on_complete)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if there's already an active job for this plan
    for (const auto& [id, job] : m_jobs) {
        if (job->get_plan_id() == plan_id) {
            auto status = job->get_status();
            if (status == JobStatus::Running || status == JobStatus::Pending) {
                return "";  // Already running
            }
        }
    }

    // Generate job ID
    std::stringstream ss;
    ss << "job_" << std::setfill('0') << std::setw(6) << m_next_job_id++;
    std::string job_id = ss.str();

    // Create and start job
    auto job = std::make_unique<OptimizationJob>(plan_id, plan);
    job->start(std::move(on_progress), std::move(on_complete));

    m_jobs[job_id] = std::move(job);

    return job_id;
}

bool JobManager::stop_job(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        it->second->stop();
        return true;
    }
    return false;
}

OptimizationJob* JobManager::get_job(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::string JobManager::get_active_job_for_plan(const std::string& plan_id) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [id, job] : m_jobs) {
        if (job->get_plan_id() == plan_id) {
            auto status = job->get_status();
            if (status == JobStatus::Running || status == JobStatus::Pending) {
                return id;
            }
        }
    }
    return "";
}

void JobManager::cleanup_completed() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto it = m_jobs.begin(); it != m_jobs.end(); ) {
        auto status = it->second->get_status();
        if (status == JobStatus::Completed ||
            status == JobStatus::Cancelled ||
            status == JobStatus::Failed) {
            it = m_jobs.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace brimstone
