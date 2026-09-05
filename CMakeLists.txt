cmake_minimum_required(VERSION 3.21)

# dH / Brimstone: variational inverse planning for radiotherapy.
#
# One CMake tree for the live build (RtModel static lib, Graph static lib,
# Brimstone MFC exe) that Brimstone_src.sln + the three .vcxproj files used to
# describe by hand. Salvaged from PR #46 (copilot/add-cmake-support-to-projects)
# and brought in line with the vcxproj settings as of 2026-09: vcpkg ITK 5.4 /
# DCMTK / WebView2, Intel oneAPI IPP, C++17, Unicode, dynamic MFC.
#
# Configure with a preset (see CMakePresets.json), e.g.
#   cmake --preset vs2022-x64
#   cmake --build --preset vs2022-x64-release
# The presets point CMAKE_TOOLCHAIN_FILE at $VCPKG_ROOT/scripts/buildsystems/
# vcpkg.cmake. The default preset uses the classic installed tree
# (C:\vcpkg\installed\x64-windows) so a machine that already builds the .sln
# needs nothing new; the *-manifest preset builds the deps from vcpkg.json.

project(dH VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DH_BUILD_BRIMSTONE "Build the Brimstone MFC GUI (needs MSVC + MFC)" ON)
option(DH_USE_IPP "Use Intel IPP (oneAPI) for vector/convolution kernels" ON)
option(DH_BUILD_PYTHON "Build the rtmodel_core pybind11 module against the RtModel target" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT MSVC)
    message(FATAL_ERROR
        "dH currently builds only with MSVC: RtModel/Graph/Brimstone use MFC "
        "(CString/CArray) and Intel IPP. A non-MFC RtModel is roadmap work "
        "(ROADMAP.md section 0).")
endif()

# --- toolchain-wide settings shared by the three targets (from the vcxprojs) ---
# Dynamic MFC for every target (RtModel/Graph use CString etc. too).
set(CMAKE_MFC_FLAG 2)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_compile_definitions(
    WIN32
    UNICODE
    _UNICODE
    NOMINMAX
    USE_RTOPT
)
# /Zc:__cplusplus: report the real standard so ITK/VXL pick C++17 paths.
# /MP: parallel compilation within a target.
add_compile_options(/Zc:__cplusplus /MP)

# --- dependencies ------------------------------------------------------------
# ITK (vcpkg port "itk"): all modules; RtModel uses Common, ImageGrid,
# ImageFunction, Transform, RegistrationCommon (pyramid), SpatialObjects,
# IOXML, Path.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# VXL (vcpkg port "vxl"): vcpkg's ITK does not ship vnl headers, so the
# <vnl/...> includes in RtModel come from the standalone port (the vcxprojs
# list include/vxl/core and include/vxl/vcl). VXLTargets.cmake exports the
# imported targets vnl, vnl_algo, vcl, v3p_netlib.
find_package(VXL REQUIRED)
set(DH_VXL_INCLUDE_DIRS "")
foreach(_d ${VXL_CORE_INCLUDE_DIR} ${VXL_VCL_INCLUDE_DIR})
    if(_d AND EXISTS "${_d}")
        list(APPEND DH_VXL_INCLUDE_DIRS "${_d}")
    endif()
endforeach()
message(STATUS "dH: VXL include dirs: ${DH_VXL_INCLUDE_DIRS}")

# Intel IPP (no vcpkg port): cmake/FindIPP.cmake looks at oneAPI, then a
# NuGet package. Brimstone.vcxproj links ipps + ippcore only.
set(DH_IPP_FOUND FALSE)
if(DH_USE_IPP)
    find_package(IPP COMPONENTS ipps ippcore)
    if(IPP_FOUND)
        set(DH_IPP_FOUND TRUE)
        message(STATUS "dH: Intel IPP ${IPP_VERSION} at ${IPP_INCLUDE_DIRS}")
    else()
        message(WARNING "dH: DH_USE_IPP=ON but Intel IPP was not found; building without USE_IPP")
    endif()
endif()

# --- targets -----------------------------------------------------------------
add_subdirectory(RtModel)
add_subdirectory(Graph)

if(DH_BUILD_BRIMSTONE)
    add_subdirectory(Brimstone)
endif()

if(DH_BUILD_PYTHON)
    # python/CMakeLists.txt still re-globs RtModel sources; wiring it to the
    # RtModel target is execution step 3 in ROADMAP.md section 0.
    add_subdirectory(python)
endif()
