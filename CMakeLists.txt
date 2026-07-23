cmake_minimum_required(VERSION 3.16)

project(dH VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Policy settings for better CMake behavior
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)  # MSVC runtime library flags
endif()

# Build options
option(BUILD_BRIMSTONE_APP "Build Brimstone GUI application (main production system)" ON)
option(BUILD_FOUNDATION_LIBS "Build foundation libraries (MTL, FTL, etc.)" OFF)
option(BUILD_COMPONENT_LIBS "Build component libraries (GEOM_MODEL, OPTIMIZER_BASE, etc.)" OFF)
option(BUILD_TESTS "Build test applications" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add cmake directory to module path for custom find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find Intel Performance Primitives (IPP) - TEMPORARILY DISABLED
# Issue: IPP headers needed in VectorOps.h (via stdafx.h) before compiler include paths set
# Note: ippm (matrix operations) was replaced by ippvm in newer IPP versions
# find_package(IPP COMPONENTS ippi ipps ippcore)
set(IPP_FOUND FALSE)
message(STATUS "Intel IPP temporarily disabled - building without optimizations")
# if(IPP_FOUND)
#     message(STATUS "Intel IPP found:")
#     message(STATUS "  Version: ${IPP_VERSION}")
#     message(STATUS "  Include dirs: ${IPP_INCLUDE_DIRS}")
#     message(STATUS "  Libraries: ${IPP_LIBRARIES}")
# else()
#     message(WARNING "Intel IPP not found. Some optimizations will be disabled.")
#     message(WARNING "Install IPP via NuGet package 'intelipp.devel.win-x64' or Intel oneAPI.")
# endif()

# Find ITK - required for main Brimstone system
# Supports three modes:
#   1. Standard find_package(ITK) with a proper CMake installation
#   2. Manual ITK_SRC_DIR + ITK_BUILD_DIR for build-tree installations
#   3. Auto-detection of common local paths
if(BUILD_BRIMSTONE_APP OR BUILD_COMPONENT_LIBS)
    set(ITK_FOUND FALSE)

    # Option: Manual ITK paths (for build-tree ITK installations)
    set(ITK_SRC_DIR "" CACHE PATH "Path to ITK source tree (contains Modules/)")
    set(ITK_BUILD_DIR "" CACHE PATH "Path to ITK build tree (contains lib/)")

    # Try standard find_package first (works with properly installed ITK)
    if(NOT ITK_SRC_DIR AND NOT ITK_BUILD_DIR)
        find_package(ITK QUIET COMPONENTS
            ITKCommon
            ITKIOImageBase
            ITKIOXML
            ITKImageGrid
            ITKImageFilterBase
            ITKImageFunction
            ITKImageCompose
            ITKSmoothing
            ITKTransform
            ITKRegistrationCommon
            ITKPath
            ITKSpatialObjects
            ITKStatistics
        )
        if(ITK_FOUND)
            include(${ITK_USE_FILE})
            message(STATUS "ITK found via find_package: ${ITK_DIR}")
        endif()
    endif()

    # Fall back to manual ITK detection
    if(NOT ITK_FOUND)
        # Auto-detect common local paths if not explicitly set
        if(NOT ITK_SRC_DIR)
            foreach(_candidate
                "C:/kits/install/InsightToolkit-5.3.0"
                "C:/kits/InsightToolkit-5.3.0"
                "$ENV{ITK_DIR}"
            )
                if(EXISTS "${_candidate}/Modules/Core/Common/include")
                    set(ITK_SRC_DIR "${_candidate}" CACHE PATH "Path to ITK source tree" FORCE)
                    message(STATUS "Auto-detected ITK source at: ${ITK_SRC_DIR}")
                    break()
                endif()
            endforeach()
        endif()

        if(NOT ITK_BUILD_DIR)
            foreach(_candidate
                "C:/kits/install/InsightToolkit-5.3.0_vc2022x64"
                "C:/kits/InsightToolkit-5.3.0_vc2022x64"
                "$ENV{ITK_BUILD_DIR}"
            )
                if(EXISTS "${_candidate}/lib")
                    set(ITK_BUILD_DIR "${_candidate}" CACHE PATH "Path to ITK build tree" FORCE)
                    message(STATUS "Auto-detected ITK build at: ${ITK_BUILD_DIR}")
                    break()
                endif()
            endforeach()
        endif()

        include(FindITKManual)
        if(ITK_MANUAL_FOUND)
            set(ITK_FOUND TRUE)
            set(ITK_USE_MANUAL TRUE)
        endif()
    endif()

    if(NOT ITK_FOUND)
        message(FATAL_ERROR
            "ITK not found. Please set one of:\n"
            "  -DITK_DIR=<path>  (for installed ITK with working CMake config)\n"
            "  -DITK_SRC_DIR=<source> -DITK_BUILD_DIR=<build>  (for build-tree ITK)"
        )
    endif()
endif()

# Find MFC if building the Brimstone application
if(BUILD_BRIMSTONE_APP)
    if(MSVC)
        # Enable MFC
        set(CMAKE_MFC_FLAG 2)  # Use MFC in a shared DLL
    else()
        message(WARNING "Brimstone requires MFC which is only available on Windows with MSVC")
        set(BUILD_BRIMSTONE_APP OFF)
    endif()
endif()

# Add subdirectories for foundation libraries
if(BUILD_FOUNDATION_LIBS)
    # Foundation template libraries
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/MTL/CMakeLists.txt)
        add_subdirectory(MTL)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/FTL/CMakeLists.txt)
        add_subdirectory(FTL)
    endif()
endif()

# Add XMLLogging library (required by Brimstone)
if(BUILD_BRIMSTONE_APP OR BUILD_COMPONENT_LIBS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/XMLLogging/CMakeLists.txt)
        add_subdirectory(XMLLogging)
    endif()
endif()

# Add subdirectories for component libraries
if(BUILD_COMPONENT_LIBS)
    # Component libraries
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/GEOM_MODEL/CMakeLists.txt)
        add_subdirectory(GEOM_MODEL)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/OptimizeN/CMakeLists.txt)
        add_subdirectory(OptimizeN)
    endif()
endif()

# Add subdirectories for main production system (Brimstone_src.sln)
# RtModel and Graph require ITK, so only build them when needed
if(BUILD_BRIMSTONE_APP OR BUILD_COMPONENT_LIBS)
    add_subdirectory(RtModel)
    add_subdirectory(Graph)
endif()

if(BUILD_BRIMSTONE_APP)
    add_subdirectory(Brimstone)
endif()

# Installation rules
if(BUILD_BRIMSTONE_APP OR BUILD_COMPONENT_LIBS)
    install(TARGETS RtModel Graph
        EXPORT BrimstoneTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

if(BUILD_BRIMSTONE_APP)
    install(TARGETS Brimstone
        RUNTIME DESTINATION bin
    )
endif()

# Export targets for use by other projects
if(BUILD_BRIMSTONE_APP OR BUILD_COMPONENT_LIBS)
    install(EXPORT BrimstoneTargets
        FILE BrimstoneTargets.cmake
        NAMESPACE Brimstone::
        DESTINATION lib/cmake/Brimstone
    )
    
    # Create and install package config file
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BrimstoneConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/BrimstoneConfig.cmake
        INSTALL_DESTINATION lib/cmake/Brimstone
    )
    
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/BrimstoneConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/BrimstoneConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/BrimstoneConfigVersion.cmake
        DESTINATION lib/cmake/Brimstone
    )
endif()
