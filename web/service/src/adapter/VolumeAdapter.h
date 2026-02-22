#pragma once

#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

// Forward declarations for dH/ITK types
namespace dH {
    class Plan;
    class Series;
    class Structure;
}
namespace itk {
    template<typename T, unsigned int D> class Image;
}

namespace brimstone {

using json = nlohmann::json;

// Volume types used in dH
using VolumeReal = itk::Image<float, 3>;

// Adapter for extracting slices and volume data
class VolumeAdapter {
public:
    // CT slice extraction
    static std::vector<uint8_t> get_ct_slice(
        dH::Plan* plan,
        int slice_index,
        float window_center = 40.0f,
        float window_width = 400.0f);

    // Dose slice extraction
    static std::vector<uint8_t> get_dose_slice(
        dH::Plan* plan,
        int slice_index,
        float max_dose = 0.0f);  // 0 = auto-detect max

    // Structure mask slice extraction
    static std::vector<uint8_t> get_structure_slice(
        dH::Structure* structure,
        int slice_index);

    // Combined CT + dose + structure overlay
    static std::vector<uint8_t> get_composite_slice(
        dH::Plan* plan,
        int slice_index,
        float window_center = 40.0f,
        float window_width = 400.0f,
        float max_dose = 0.0f,
        const std::vector<std::string>& visible_structures = {});

    // Volume info
    static json get_ct_info(dH::Plan* plan);
    static json get_dose_info(dH::Plan* plan);
    static json get_volume_bounds(dH::Plan* plan);

    // Full volume data (for 3D rendering)
    static std::vector<uint8_t> get_ct_volume(
        dH::Plan* plan,
        int downsample = 1);  // 1 = full resolution, 2 = half, etc.

    static std::vector<uint8_t> get_dose_volume(
        dH::Plan* plan,
        int downsample = 1,
        float max_dose = 0.0f);

private:
    // Find max dose in volume for normalization
    static float find_max_dose(VolumeReal* dose);
};

} // namespace brimstone
