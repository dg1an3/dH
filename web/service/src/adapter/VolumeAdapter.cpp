#include "VolumeAdapter.h"
#include "../util/BinaryWriter.h"
#include "../util/JsonUtils.h"

#include <Plan.h>
#include <Series.h>
#include <Structure.h>

#include <itkImage.h>
#include <itkImageRegionConstIterator.h>

#include <algorithm>
#include <cstring>

namespace brimstone {

using namespace json_utils;
using namespace binary;

std::vector<uint8_t> VolumeAdapter::get_ct_slice(
    dH::Plan* plan,
    int slice_index,
    float window_center,
    float window_width)
{
    if (!plan || !plan->GetSeries()) {
        return {};
    }

    VolumeReal* density = plan->GetSeries()->GetDensity();
    if (!density) {
        return {};
    }

    return extract_slice(density, slice_index, window_center, window_width);
}

std::vector<uint8_t> VolumeAdapter::get_dose_slice(
    dH::Plan* plan,
    int slice_index,
    float max_dose)
{
    if (!plan) {
        return {};
    }

    VolumeReal* dose = plan->GetDoseMatrix();
    if (!dose) {
        return {};
    }

    // Auto-detect max dose if not specified
    if (max_dose <= 0.0f) {
        max_dose = find_max_dose(dose);
    }

    return extract_dose_slice(dose, slice_index, max_dose);
}

std::vector<uint8_t> VolumeAdapter::get_structure_slice(
    dH::Structure* structure,
    int slice_index)
{
    if (!structure) {
        return {};
    }

    // Get the structure region mask at level 0 (highest resolution)
    const VolumeReal* region = structure->GetRegion(0);
    if (!region) {
        return {};
    }

    return extract_slice(region, slice_index, 0.5f, 1.0f);
}

std::vector<uint8_t> VolumeAdapter::get_composite_slice(
    dH::Plan* plan,
    int slice_index,
    float window_center,
    float window_width,
    float max_dose,
    const std::vector<std::string>& visible_structures)
{
    // For now, just return CT slice - composite rendering done client-side
    // by fetching separate layers
    return get_ct_slice(plan, slice_index, window_center, window_width);
}

json VolumeAdapter::get_ct_info(dH::Plan* plan) {
    if (!plan || !plan->GetSeries()) {
        return json{{"error", "no series"}};
    }

    VolumeReal* density = plan->GetSeries()->GetDensity();
    if (!density) {
        return json{{"error", "no density volume"}};
    }

    json result = image_info_to_json(density);
    result["type"] = "ct";

    // Add slice count for easy access
    auto size = density->GetLargestPossibleRegion().GetSize();
    result["sliceCount"] = size[2];

    return result;
}

json VolumeAdapter::get_dose_info(dH::Plan* plan) {
    if (!plan) {
        return json{{"error", "no plan"}};
    }

    VolumeReal* dose = plan->GetDoseMatrix();
    if (!dose) {
        return json{{"error", "no dose calculated"}};
    }

    json result = image_info_to_json(dose);
    result["type"] = "dose";
    result["maxDose"] = find_max_dose(dose);
    result["resolution"] = plan->GetDoseResolution();

    auto size = dose->GetLargestPossibleRegion().GetSize();
    result["sliceCount"] = size[2];

    return result;
}

json VolumeAdapter::get_volume_bounds(dH::Plan* plan) {
    if (!plan || !plan->GetSeries()) {
        return json{{"error", "no series"}};
    }

    VolumeReal* density = plan->GetSeries()->GetDensity();
    if (!density) {
        return json{{"error", "no density volume"}};
    }

    auto region = density->GetLargestPossibleRegion();
    auto size = region.GetSize();
    auto spacing = density->GetSpacing();
    auto origin = density->GetOrigin();

    // Calculate physical bounds
    double min_x = origin[0];
    double min_y = origin[1];
    double min_z = origin[2];
    double max_x = origin[0] + (size[0] - 1) * spacing[0];
    double max_y = origin[1] + (size[1] - 1) * spacing[1];
    double max_z = origin[2] + (size[2] - 1) * spacing[2];

    return json{
        {"min", {min_x, min_y, min_z}},
        {"max", {max_x, max_y, max_z}},
        {"center", {(min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2}}
    };
}

std::vector<uint8_t> VolumeAdapter::get_ct_volume(
    dH::Plan* plan,
    int downsample)
{
    if (!plan || !plan->GetSeries()) {
        return {};
    }

    VolumeReal* density = plan->GetSeries()->GetDensity();
    if (!density) {
        return {};
    }

    auto region = density->GetLargestPossibleRegion();
    auto size = region.GetSize();
    auto spacing = density->GetSpacing();
    auto origin = density->GetOrigin();

    // Calculate downsampled dimensions
    int ds = std::max(1, downsample);
    uint32_t width = static_cast<uint32_t>((size[0] + ds - 1) / ds);
    uint32_t height = static_cast<uint32_t>((size[1] + ds - 1) / ds);
    uint32_t depth = static_cast<uint32_t>((size[2] + ds - 1) / ds);

    // Header: dimensions (3 uint32), spacing (3 float), origin (3 float)
    size_t header_size = 3 * sizeof(uint32_t) + 6 * sizeof(float);
    size_t data_size = width * height * depth * sizeof(float);
    std::vector<uint8_t> buffer(header_size + data_size);

    // Write header
    uint32_t* dims = reinterpret_cast<uint32_t*>(buffer.data());
    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;

    float* spacing_out = reinterpret_cast<float*>(buffer.data() + 3 * sizeof(uint32_t));
    spacing_out[0] = static_cast<float>(spacing[0] * ds);
    spacing_out[1] = static_cast<float>(spacing[1] * ds);
    spacing_out[2] = static_cast<float>(spacing[2] * ds);

    float* origin_out = reinterpret_cast<float*>(buffer.data() + 3 * sizeof(uint32_t) + 3 * sizeof(float));
    origin_out[0] = static_cast<float>(origin[0]);
    origin_out[1] = static_cast<float>(origin[1]);
    origin_out[2] = static_cast<float>(origin[2]);

    // Copy downsampled data
    float* voxels = reinterpret_cast<float*>(buffer.data() + header_size);

    itk::Index<3> idx;
    size_t out_idx = 0;
    for (uint32_t z = 0; z < depth; ++z) {
        idx[2] = std::min(static_cast<size_t>(z * ds), size[2] - 1);
        for (uint32_t y = 0; y < height; ++y) {
            idx[1] = std::min(static_cast<size_t>(y * ds), size[1] - 1);
            for (uint32_t x = 0; x < width; ++x) {
                idx[0] = std::min(static_cast<size_t>(x * ds), size[0] - 1);
                voxels[out_idx++] = density->GetPixel(idx);
            }
        }
    }

    return buffer;
}

std::vector<uint8_t> VolumeAdapter::get_dose_volume(
    dH::Plan* plan,
    int downsample,
    float max_dose)
{
    if (!plan) {
        return {};
    }

    VolumeReal* dose = plan->GetDoseMatrix();
    if (!dose) {
        return {};
    }

    if (max_dose <= 0.0f) {
        max_dose = find_max_dose(dose);
    }

    auto region = dose->GetLargestPossibleRegion();
    auto size = region.GetSize();
    auto spacing = dose->GetSpacing();
    auto origin = dose->GetOrigin();

    // Calculate downsampled dimensions
    int ds = std::max(1, downsample);
    uint32_t width = static_cast<uint32_t>((size[0] + ds - 1) / ds);
    uint32_t height = static_cast<uint32_t>((size[1] + ds - 1) / ds);
    uint32_t depth = static_cast<uint32_t>((size[2] + ds - 1) / ds);

    // Header: dimensions (3 uint32), spacing (3 float), origin (3 float), max_dose (1 float)
    size_t header_size = 3 * sizeof(uint32_t) + 7 * sizeof(float);
    size_t data_size = width * height * depth * sizeof(float);
    std::vector<uint8_t> buffer(header_size + data_size);

    // Write header
    uint32_t* dims = reinterpret_cast<uint32_t*>(buffer.data());
    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;

    float* header_floats = reinterpret_cast<float*>(buffer.data() + 3 * sizeof(uint32_t));
    header_floats[0] = static_cast<float>(spacing[0] * ds);
    header_floats[1] = static_cast<float>(spacing[1] * ds);
    header_floats[2] = static_cast<float>(spacing[2] * ds);
    header_floats[3] = static_cast<float>(origin[0]);
    header_floats[4] = static_cast<float>(origin[1]);
    header_floats[5] = static_cast<float>(origin[2]);
    header_floats[6] = max_dose;

    // Copy downsampled and normalized data
    float* voxels = reinterpret_cast<float*>(buffer.data() + header_size);
    float scale = (max_dose > 0.0f) ? (1.0f / max_dose) : 1.0f;

    itk::Index<3> idx;
    size_t out_idx = 0;
    for (uint32_t z = 0; z < depth; ++z) {
        idx[2] = std::min(static_cast<size_t>(z * ds), size[2] - 1);
        for (uint32_t y = 0; y < height; ++y) {
            idx[1] = std::min(static_cast<size_t>(y * ds), size[1] - 1);
            for (uint32_t x = 0; x < width; ++x) {
                idx[0] = std::min(static_cast<size_t>(x * ds), size[0] - 1);
                float val = dose->GetPixel(idx) * scale;
                voxels[out_idx++] = std::min(val, 1.0f);
            }
        }
    }

    return buffer;
}

float VolumeAdapter::find_max_dose(VolumeReal* dose) {
    if (!dose) {
        return 1.0f;
    }

    float max_val = 0.0f;

    using IteratorType = itk::ImageRegionConstIterator<VolumeReal>;
    IteratorType it(dose, dose->GetLargestPossibleRegion());

    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        float val = it.Get();
        if (val > max_val) {
            max_val = val;
        }
    }

    return (max_val > 0.0f) ? max_val : 1.0f;
}

} // namespace brimstone
