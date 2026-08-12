#pragma once

#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

// Forward declarations for dH types
namespace dH {
    class Plan;
    class Series;
    class Beam;
    class Structure;
    class KLDivTerm;
}
class CHistogram;

namespace brimstone {

using json = nlohmann::json;

// Adapter for converting Plan objects to/from JSON
class PlanAdapter {
public:
    // Plan serialization
    static json plan_to_json(dH::Plan* plan);
    static json plan_summary_to_json(dH::Plan* plan, const std::string& id);

    // Component serialization
    static json beam_to_json(dH::Beam* beam, int index);
    static json structure_to_json(dH::Structure* structure);
    static json histogram_to_json(CHistogram* histogram, const std::string& structure_name);

    // Structure type string conversion
    static std::string structure_type_to_string(int type);
    static int string_to_structure_type(const std::string& str);

    // Prescription/objective serialization
    static json kldiv_term_to_json(dH::KLDivTerm* term, dH::Structure* structure);

    // Deserialization (for updates)
    static void update_beam_from_json(dH::Beam* beam, const json& data);
    static void update_structure_from_json(dH::Structure* structure, const json& data);
};

// Plan store - manages loaded plans with thread-safe access
class PlanStore {
public:
    static PlanStore& instance();

    // Plan management
    std::string add_plan(dH::Plan* plan, const std::string& name = "");
    dH::Plan* get_plan(const std::string& id);
    bool remove_plan(const std::string& id);
    std::vector<std::string> get_plan_ids() const;

    // Plan loading
    std::string load_plan_from_xml(const std::string& xml_path);
    std::string create_empty_plan(const std::string& name);

private:
    PlanStore() = default;
    ~PlanStore() = default;
    PlanStore(const PlanStore&) = delete;
    PlanStore& operator=(const PlanStore&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::pair<std::string, dH::Plan*>> m_plans;  // id -> (name, plan)
    int m_next_id = 1;

    std::string generate_id();
};

} // namespace brimstone
