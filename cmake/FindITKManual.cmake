# FindITKManual.cmake
# -------------------
# Manual ITK configuration for build-tree ITK installations where
# ITKConfig.cmake has hardcoded/broken paths.
#
# This module sets up ITK include directories and libraries by directly
# scanning the ITK source tree (ITK_SRC_DIR) and build tree (ITK_BUILD_DIR).
#
# Input Variables:
#   ITK_SRC_DIR   - Path to ITK source tree (contains Modules/)
#   ITK_BUILD_DIR - Path to ITK build tree (contains lib/, Modules/ with generated headers)
#
# Result Variables:
#   ITK_MANUAL_FOUND        - True if ITK was found manually
#   ITK_MANUAL_INCLUDE_DIRS - Include directories
#   ITK_MANUAL_LIBRARIES    - Libraries to link against

if(NOT ITK_SRC_DIR OR NOT ITK_BUILD_DIR)
    message(STATUS "FindITKManual: ITK_SRC_DIR and ITK_BUILD_DIR not both set, skipping manual detection")
    set(ITK_MANUAL_FOUND FALSE)
    return()
endif()

# Verify directories exist
if(NOT EXISTS "${ITK_SRC_DIR}/Modules")
    message(WARNING "FindITKManual: ITK_SRC_DIR '${ITK_SRC_DIR}' does not contain Modules/")
    set(ITK_MANUAL_FOUND FALSE)
    return()
endif()

if(NOT EXISTS "${ITK_BUILD_DIR}/lib")
    message(WARNING "FindITKManual: ITK_BUILD_DIR '${ITK_BUILD_DIR}' does not contain lib/")
    set(ITK_MANUAL_FOUND FALSE)
    return()
endif()

message(STATUS "FindITKManual: Using ITK source at ${ITK_SRC_DIR}")
message(STATUS "FindITKManual: Using ITK build at ${ITK_BUILD_DIR}")

# Collect include directories matching what the vcxproj uses
set(ITK_MANUAL_INCLUDE_DIRS
    # Source tree module includes
    "${ITK_SRC_DIR}/Modules/Core/Common/include"
    "${ITK_SRC_DIR}/Modules/Core/ImageFunction/include"
    "${ITK_SRC_DIR}/Modules/Core/SpatialObjects/include"
    "${ITK_SRC_DIR}/Modules/Core/Transform/include"
    "${ITK_SRC_DIR}/Modules/Filtering/ImageFilterBase/include"
    "${ITK_SRC_DIR}/Modules/Filtering/ImageGrid/include"
    "${ITK_SRC_DIR}/Modules/Filtering/ImageCompose/include"
    "${ITK_SRC_DIR}/Modules/Filtering/Smoothing/include"
    "${ITK_SRC_DIR}/Modules/Filtering/Path/include"
    "${ITK_SRC_DIR}/Modules/IO/ImageBase/include"
    "${ITK_SRC_DIR}/Modules/IO/XML/include"
    "${ITK_SRC_DIR}/Modules/Numerics/Statistics/include"
    "${ITK_SRC_DIR}/Modules/Registration/Common/include"
    # Third party includes from source tree
    "${ITK_SRC_DIR}/Modules/ThirdParty/VNL/src/vxl/vcl"
    "${ITK_SRC_DIR}/Modules/ThirdParty/VNL/src/vxl/core"
    "${ITK_SRC_DIR}/Modules/ThirdParty/Expat/src/expat"
    # Build tree generated includes
    "${ITK_BUILD_DIR}/Modules/Core/Common"
    "${ITK_BUILD_DIR}/Modules/IO/ImageBase"
    "${ITK_BUILD_DIR}/Modules/ThirdParty/VNL/src/vxl/vcl"
    "${ITK_BUILD_DIR}/Modules/ThirdParty/VNL/src/vxl/core"
    "${ITK_BUILD_DIR}/Modules/ThirdParty/Expat/src/expat"
    "${ITK_BUILD_DIR}/Modules/ThirdParty/KWSys/src"
    # Eigen3 (required by itkSymmetricEigenAnalysis.h)
    "${ITK_BUILD_DIR}/Modules/ThirdParty/Eigen3/src"
    "${ITK_SRC_DIR}/Modules/ThirdParty/Eigen3/src"
    "${ITK_SRC_DIR}/Modules/ThirdParty/Eigen3/src/itkeigen"
)

# Detect ITK version from source
set(ITK_MANUAL_VERSION "5.3")
if(EXISTS "${ITK_SRC_DIR}/CMakeLists.txt")
    file(STRINGS "${ITK_SRC_DIR}/CMakeLists.txt" _itk_version_line REGEX "^set\\(ITK_VERSION_MAJOR")
    if(_itk_version_line)
        string(REGEX MATCH "[0-9]+" ITK_MANUAL_VERSION_MAJOR "${_itk_version_line}")
    endif()
    file(STRINGS "${ITK_SRC_DIR}/CMakeLists.txt" _itk_version_line REGEX "^set\\(ITK_VERSION_MINOR")
    if(_itk_version_line)
        string(REGEX MATCH "[0-9]+" ITK_MANUAL_VERSION_MINOR "${_itk_version_line}")
    endif()
    if(ITK_MANUAL_VERSION_MAJOR AND ITK_MANUAL_VERSION_MINOR)
        set(ITK_MANUAL_VERSION "${ITK_MANUAL_VERSION_MAJOR}.${ITK_MANUAL_VERSION_MINOR}")
    endif()
endif()

message(STATUS "FindITKManual: Detected ITK version ${ITK_MANUAL_VERSION}")

# Find libraries - these are the core ITK libs needed by brimstone
# The vcxproj links: ITKCommon, itkvnl_algo, itkv3p_netlib, itkvnl, itkvcl, itksys
set(_itk_lib_suffix "-${ITK_MANUAL_VERSION}")
set(_itk_required_libs
    "ITKCommon${_itk_lib_suffix}"
    "ITKSpatialObjects${_itk_lib_suffix}"
    "ITKStatistics${_itk_lib_suffix}"
    "ITKPath${_itk_lib_suffix}"
    "itkvnl_algo${_itk_lib_suffix}"
    "itkvnl${_itk_lib_suffix}"
    "itkvcl${_itk_lib_suffix}"
    "itksys${_itk_lib_suffix}"
    "ITKVNLInstantiation${_itk_lib_suffix}"
    "itkv3p_netlib${_itk_lib_suffix}"
    "itkdouble-conversion${_itk_lib_suffix}"
)

set(ITK_MANUAL_LIBRARIES "")
set(_itk_all_found TRUE)

# Determine library directory - check for config-specific subdirs
set(_itk_lib_search_paths
    "${ITK_BUILD_DIR}/lib"
)

# Check for configuration-specific library directories (Debug, Release, etc.)
foreach(_config Debug Release RelWithDebInfo MinSizeRel)
    if(EXISTS "${ITK_BUILD_DIR}/lib/${_config}")
        list(APPEND _itk_lib_search_paths "${ITK_BUILD_DIR}/lib/${_config}")
    endif()
endforeach()

foreach(_lib ${_itk_required_libs})
    find_library(ITK_${_lib}_LIBRARY
        NAMES ${_lib}
        PATHS ${_itk_lib_search_paths}
        NO_DEFAULT_PATH
    )
    if(ITK_${_lib}_LIBRARY)
        list(APPEND ITK_MANUAL_LIBRARIES ${ITK_${_lib}_LIBRARY})
    else()
        message(STATUS "FindITKManual: Optional library ${_lib} not found (may not be needed)")
    endif()
endforeach()

# Check that at least ITKCommon was found
if(NOT ITK_ITKCommon${_itk_lib_suffix}_LIBRARY)
    message(WARNING "FindITKManual: Could not find ITKCommon library")
    set(ITK_MANUAL_FOUND FALSE)
    return()
endif()

set(ITK_MANUAL_FOUND TRUE)

# Create imported target
if(NOT TARGET ITK::Manual)
    add_library(ITK::Manual INTERFACE IMPORTED)
    set_target_properties(ITK::Manual PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ITK_MANUAL_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${ITK_MANUAL_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "/bigobj"
    )
endif()

# Also set ITK_LIBRARIES and ITK_INCLUDE_DIRS for compatibility
set(ITK_LIBRARIES ${ITK_MANUAL_LIBRARIES})
set(ITK_INCLUDE_DIRS ${ITK_MANUAL_INCLUDE_DIRS})

message(STATUS "FindITKManual: Found ITK ${ITK_MANUAL_VERSION}")
message(STATUS "FindITKManual: Include dirs: ${ITK_MANUAL_INCLUDE_DIRS}")
message(STATUS "FindITKManual: Libraries: ${ITK_MANUAL_LIBRARIES}")
