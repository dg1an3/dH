#pragma once

#include <nlohmann/json.hpp>
#include <itkImage.h>
#include <itkVector.h>
#include <string>
#include <vector>

namespace brimstone {
namespace json_utils {

using json = nlohmann::json;

// ITK Vector serialization
template<typename T, unsigned int N>
json to_json(const itk::Vector<T, N>& vec) {
    json arr = json::array();
    for (unsigned int i = 0; i < N; ++i) {
        arr.push_back(vec[i]);
    }
    return arr;
}

template<typename T, unsigned int N>
itk::Vector<T, N> from_json_vector(const json& arr) {
    itk::Vector<T, N> vec;
    for (unsigned int i = 0; i < N && i < arr.size(); ++i) {
        vec[i] = arr[i].get<T>();
    }
    return vec;
}

// ITK Index serialization
template<unsigned int N>
json to_json(const itk::Index<N>& idx) {
    json arr = json::array();
    for (unsigned int i = 0; i < N; ++i) {
        arr.push_back(idx[i]);
    }
    return arr;
}

// ITK Size serialization
template<unsigned int N>
json to_json(const itk::Size<N>& sz) {
    json arr = json::array();
    for (unsigned int i = 0; i < N; ++i) {
        arr.push_back(sz[i]);
    }
    return arr;
}

// ITK Point serialization
template<typename T, unsigned int N>
json to_json(const itk::Point<T, N>& pt) {
    json arr = json::array();
    for (unsigned int i = 0; i < N; ++i) {
        arr.push_back(pt[i]);
    }
    return arr;
}

// Image region info
template<typename ImageType>
json image_info_to_json(const ImageType* image) {
    if (!image) {
        return json{{"error", "null image"}};
    }

    auto region = image->GetLargestPossibleRegion();
    auto spacing = image->GetSpacing();
    auto origin = image->GetOrigin();

    return json{
        {"dimensions", to_json(region.GetSize())},
        {"spacing", to_json(spacing)},
        {"origin", to_json(origin)}
    };
}

// Color conversion (COLORREF is BGR format)
inline json colorref_to_json(unsigned long colorref) {
    return json{
        {"r", (colorref & 0xFF)},
        {"g", ((colorref >> 8) & 0xFF)},
        {"b", ((colorref >> 16) & 0xFF)}
    };
}

inline unsigned long json_to_colorref(const json& color) {
    int r = color.value("r", 0);
    int g = color.value("g", 0);
    int b = color.value("b", 0);
    return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
}

} // namespace json_utils
} // namespace brimstone
