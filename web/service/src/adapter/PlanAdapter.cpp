#include "PlanAdapter.h"
#include "../util/JsonUtils.h"

#include <Plan.h>
#include <Series.h>
#include <Beam.h>
#include <Structure.h>
#include <Histogram.h>
#include <KLDivTerm.h>
#include <PlanXmlFile.h>

#include <sstream>
#include <iomanip>
#include <chrono>

namespace brimstone {

using namespace json_utils;

json PlanAdapter::plan_to_json(dH::Plan* plan) {
    if (!plan) {
        return json{{"error", "null plan"}};
    }

    json result;

    // Basic info
    result["doseResolution"] = plan->GetDoseResolution();

    // Series info (CT volume)
    dH::Series* series = plan->GetSeries();
    if (series) {
        json series_json;
        auto* density = series->GetDensity();
        if (density) {
            series_json["density"] = image_info_to_json(density);
        }

        // Structures
        json structures = json::array();
        for (int i = 0; i < series->GetStructureCount(); ++i) {
            dH::Structure* structure = series->GetStructureAt(i);
            if (structure) {
                structures.push_back(structure_to_json(structure));
            }
        }
        series_json["structures"] = structures;
        result["series"] = series_json;
    }

    // Beams
    json beams = json::array();
    for (int i = 0; i < plan->GetBeamCount(); ++i) {
        dH::Beam* beam = plan->GetBeamAt(i);
        if (beam) {
            beams.push_back(beam_to_json(beam, i));
        }
    }
    result["beams"] = beams;
    result["totalBeamlets"] = plan->GetTotalBeamletCount();

    // Dose matrix info (if calculated)
    auto* dose = plan->GetDoseMatrix();
    if (dose) {
        result["dose"] = image_info_to_json(dose);
    }

    return result;
}

json PlanAdapter::plan_summary_to_json(dH::Plan* plan, const std::string& id) {
    json result;
    result["id"] = id;

    if (!plan) {
        result["error"] = "null plan";
        return result;
    }

    result["beamCount"] = plan->GetBeamCount();
    result["totalBeamlets"] = plan->GetTotalBeamletCount();
    result["doseResolution"] = plan->GetDoseResolution();

    dH::Series* series = plan->GetSeries();
    if (series) {
        result["structureCount"] = series->GetStructureCount();
        auto* density = series->GetDensity();
        if (density) {
            auto size = density->GetLargestPossibleRegion().GetSize();
            result["dimensions"] = to_json(size);
        }
    }

    result["hasDose"] = (plan->GetDoseMatrix() != nullptr);

    return result;
}

json PlanAdapter::beam_to_json(dH::Beam* beam, int index) {
    json result;
    result["index"] = index;

    if (!beam) {
        result["error"] = "null beam";
        return result;
    }

    result["gantryAngle"] = beam->GetGantryAngle();
    result["isocenter"] = to_json(beam->GetIsocenter());
    result["beamletCount"] = beam->GetBeamletCount();

    // Intensity map info
    auto* im = beam->GetIntensityMap();
    if (im) {
        auto size = im->GetLargestPossibleRegion().GetSize();
        result["intensityMapSize"] = size[0];
    }

    return result;
}

json PlanAdapter::structure_to_json(dH::Structure* structure) {
    json result;

    if (!structure) {
        result["error"] = "null structure";
        return result;
    }

    result["name"] = structure->GetName();
    result["type"] = structure_type_to_string(structure->GetType());
    result["priority"] = structure->GetPriority();
    result["visible"] = structure->GetVisible();
    result["color"] = colorref_to_json(structure->GetColor());
    result["contourCount"] = structure->GetContourCount();

    return result;
}

json PlanAdapter::histogram_to_json(CHistogram* histogram, const std::string& structure_name) {
    json result;
    result["structure"] = structure_name;

    if (!histogram) {
        result["error"] = "null histogram";
        return result;
    }

    result["binMinValue"] = histogram->GetBinMinValue();
    result["binWidth"] = histogram->GetBinWidth();

    // Extract cumulative DVH data
    const auto& cum_bins = histogram->GetCumBins();
    const auto& bin_means = histogram->GetBinMeans();

    json dvh_points = json::array();
    int n_bins = static_cast<int>(cum_bins.GetDim());
    for (int i = 0; i < n_bins; ++i) {
        dvh_points.push_back({
            {"dose", bin_means[i]},
            {"volume", cum_bins[i]}
        });
    }
    result["dvh"] = dvh_points;

    return result;
}

std::string PlanAdapter::structure_type_to_string(int type) {
    switch (type) {
        case dH::Structure::eTARGET: return "target";
        case dH::Structure::eOAR: return "oar";
        default: return "none";
    }
}

int PlanAdapter::string_to_structure_type(const std::string& str) {
    if (str == "target") return dH::Structure::eTARGET;
    if (str == "oar") return dH::Structure::eOAR;
    return dH::Structure::eNONE;
}

json PlanAdapter::kldiv_term_to_json(dH::KLDivTerm* term, dH::Structure* structure) {
    json result;

    if (!term || !structure) {
        result["error"] = "null term or structure";
        return result;
    }

    result["structure"] = structure->GetName();
    result["weight"] = term->GetWeight();
    result["minDose"] = term->GetMinDose();
    result["maxDose"] = term->GetMaxDose();

    // DVP points (dose-volume points)
    const auto& dvps = term->GetDVPs();
    json dvp_array = json::array();
    int rows = dvps.GetRows();
    for (int i = 0; i < rows; ++i) {
        dvp_array.push_back({
            {"dose", dvps[i][0]},
            {"volume", dvps[i][1]}
        });
    }
    result["dvpPoints"] = dvp_array;

    return result;
}

void PlanAdapter::update_beam_from_json(dH::Beam* beam, const json& data) {
    if (!beam) return;

    if (data.contains("gantryAngle")) {
        beam->SetGantryAngle(data["gantryAngle"].get<double>());
    }

    if (data.contains("isocenter")) {
        auto iso = from_json_vector<float, 3>(data["isocenter"]);
        beam->SetIsocenter(iso);
    }
}

void PlanAdapter::update_structure_from_json(dH::Structure* structure, const json& data) {
    if (!structure) return;

    if (data.contains("visible")) {
        structure->SetVisible(data["visible"].get<bool>());
    }

    if (data.contains("color")) {
        structure->SetColor(json_to_colorref(data["color"]));
    }

    if (data.contains("priority")) {
        structure->SetPriority(data["priority"].get<int>());
    }

    if (data.contains("type")) {
        structure->SetType(
            static_cast<dH::Structure::StructType>(
                string_to_structure_type(data["type"].get<std::string>())
            )
        );
    }
}

// PlanStore implementation
PlanStore& PlanStore::instance() {
    static PlanStore instance;
    return instance;
}

std::string PlanStore::generate_id() {
    std::stringstream ss;
    ss << "plan_" << std::setfill('0') << std::setw(4) << m_next_id++;
    return ss.str();
}

std::string PlanStore::add_plan(dH::Plan* plan, const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string id = generate_id();
    std::string plan_name = name.empty() ? id : name;
    m_plans[id] = std::make_pair(plan_name, plan);
    return id;
}

dH::Plan* PlanStore::get_plan(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_plans.find(id);
    if (it != m_plans.end()) {
        return it->second.second;
    }
    return nullptr;
}

bool PlanStore::remove_plan(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_plans.find(id);
    if (it != m_plans.end()) {
        // Plan uses ITK smart pointers, so just remove from map
        m_plans.erase(it);
        return true;
    }
    return false;
}

std::vector<std::string> PlanStore::get_plan_ids() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> ids;
    ids.reserve(m_plans.size());
    for (const auto& p : m_plans) {
        ids.push_back(p.first);
    }
    return ids;
}

std::string PlanStore::load_plan_from_xml(const std::string& xml_path) {
    // Use dH's XML reader
    dH::PlanXmlReader reader;
    if (!reader.CanReadFile(xml_path.c_str())) {
        return "";
    }

    reader.SetFileName(xml_path.c_str());
    reader.Read();

    dH::Plan* plan = reader.GetOutput();
    if (!plan) {
        return "";
    }

    // Extract name from path
    std::string name = xml_path;
    size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos) {
        name = name.substr(slash + 1);
    }
    size_t dot = name.find_last_of('.');
    if (dot != std::string::npos) {
        name = name.substr(0, dot);
    }

    return add_plan(plan, name);
}

std::string PlanStore::create_empty_plan(const std::string& name) {
    dH::Plan::Pointer plan = dH::Plan::New();
    dH::Series::Pointer series = dH::Series::New();
    plan->SetSeries(series);

    // Register() increments reference count so plan survives after Pointer goes out of scope
    plan->Register();

    return add_plan(plan.GetPointer(), name);
}

} // namespace brimstone
