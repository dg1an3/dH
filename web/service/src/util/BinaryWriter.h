#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <itkImage.h>

namespace brimstone {
namespace binary {

// Binary slice header for client parsing
struct SliceHeader {
    uint32_t width;
    uint32_t height;
    float spacing_x;
    float spacing_y;
    float origin_x;
    float origin_y;
    float origin_z;  // Z position of this slice
    float window_center;
    float window_width;
};

// Extract a 2D slice from a 3D volume as raw float32 data
// Returns header + pixel data
template<typename PixelType>
std::vector<uint8_t> extract_slice(
    const itk::Image<PixelType, 3>* volume,
    int slice_index,
    float window_center = 0.0f,
    float window_width = 1000.0f)
{
    if (!volume) {
        return {};
    }

    auto region = volume->GetLargestPossibleRegion();
    auto size = region.GetSize();
    auto spacing = volume->GetSpacing();
    auto origin = volume->GetOrigin();

    // Clamp slice index
    if (slice_index < 0) slice_index = 0;
    if (slice_index >= static_cast<int>(size[2])) {
        slice_index = static_cast<int>(size[2]) - 1;
    }

    uint32_t width = static_cast<uint32_t>(size[0]);
    uint32_t height = static_cast<uint32_t>(size[1]);

    // Build header
    SliceHeader header;
    header.width = width;
    header.height = height;
    header.spacing_x = static_cast<float>(spacing[0]);
    header.spacing_y = static_cast<float>(spacing[1]);
    header.origin_x = static_cast<float>(origin[0]);
    header.origin_y = static_cast<float>(origin[1]);
    header.origin_z = static_cast<float>(origin[2] + slice_index * spacing[2]);
    header.window_center = window_center;
    header.window_width = window_width;

    // Allocate output buffer
    size_t pixel_count = width * height;
    size_t data_size = sizeof(SliceHeader) + pixel_count * sizeof(float);
    std::vector<uint8_t> buffer(data_size);

    // Copy header
    std::memcpy(buffer.data(), &header, sizeof(SliceHeader));

    // Copy pixel data (convert to float)
    float* pixels = reinterpret_cast<float*>(buffer.data() + sizeof(SliceHeader));

    itk::Index<3> idx;
    idx[2] = slice_index;

    for (uint32_t y = 0; y < height; ++y) {
        idx[1] = y;
        for (uint32_t x = 0; x < width; ++x) {
            idx[0] = x;
            pixels[y * width + x] = static_cast<float>(volume->GetPixel(idx));
        }
    }

    return buffer;
}

// Extract dose slice with optional scaling
template<typename PixelType>
std::vector<uint8_t> extract_dose_slice(
    const itk::Image<PixelType, 3>* dose_volume,
    int slice_index,
    float max_dose = 1.0f)
{
    if (!dose_volume) {
        return {};
    }

    auto region = dose_volume->GetLargestPossibleRegion();
    auto size = region.GetSize();
    auto spacing = dose_volume->GetSpacing();
    auto origin = dose_volume->GetOrigin();

    // Clamp slice index
    if (slice_index < 0) slice_index = 0;
    if (slice_index >= static_cast<int>(size[2])) {
        slice_index = static_cast<int>(size[2]) - 1;
    }

    uint32_t width = static_cast<uint32_t>(size[0]);
    uint32_t height = static_cast<uint32_t>(size[1]);

    // Build header
    SliceHeader header;
    header.width = width;
    header.height = height;
    header.spacing_x = static_cast<float>(spacing[0]);
    header.spacing_y = static_cast<float>(spacing[1]);
    header.origin_x = static_cast<float>(origin[0]);
    header.origin_y = static_cast<float>(origin[1]);
    header.origin_z = static_cast<float>(origin[2] + slice_index * spacing[2]);
    header.window_center = max_dose * 0.5f;
    header.window_width = max_dose;

    // Allocate output buffer
    size_t pixel_count = width * height;
    size_t data_size = sizeof(SliceHeader) + pixel_count * sizeof(float);
    std::vector<uint8_t> buffer(data_size);

    // Copy header
    std::memcpy(buffer.data(), &header, sizeof(SliceHeader));

    // Copy pixel data normalized to [0, 1]
    float* pixels = reinterpret_cast<float*>(buffer.data() + sizeof(SliceHeader));

    itk::Index<3> idx;
    idx[2] = slice_index;

    float scale = (max_dose > 0.0f) ? (1.0f / max_dose) : 1.0f;

    for (uint32_t y = 0; y < height; ++y) {
        idx[1] = y;
        for (uint32_t x = 0; x < width; ++x) {
            idx[0] = x;
            float val = static_cast<float>(dose_volume->GetPixel(idx)) * scale;
            pixels[y * width + x] = (val > 1.0f) ? 1.0f : val;
        }
    }

    return buffer;
}

// Encode DVH data as binary (pairs of dose, volume fractions)
inline std::vector<uint8_t> encode_dvh(
    const std::vector<float>& doses,
    const std::vector<float>& volumes)
{
    size_t count = std::min(doses.size(), volumes.size());
    std::vector<uint8_t> buffer(sizeof(uint32_t) + count * 2 * sizeof(float));

    uint32_t n = static_cast<uint32_t>(count);
    std::memcpy(buffer.data(), &n, sizeof(uint32_t));

    float* data = reinterpret_cast<float*>(buffer.data() + sizeof(uint32_t));
    for (size_t i = 0; i < count; ++i) {
        data[i * 2] = doses[i];
        data[i * 2 + 1] = volumes[i];
    }

    return buffer;
}

} // namespace binary
} // namespace brimstone
