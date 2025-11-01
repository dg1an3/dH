cmake_minimum_required(VERSION 3.16)

project(Brimstone VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Policy settings for better CMake behavior
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)  # MSVC runtime library flags
endif()

# Build options
option(BUILD_BRIMSTONE_APP "Build Brimstone GUI application" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find ITK - required for RtModel
find_package(ITK REQUIRED COMPONENTS
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
include(${ITK_USE_FILE})

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

# Add subdirectories for each project
add_subdirectory(RtModel)
add_subdirectory(Graph)

if(BUILD_BRIMSTONE_APP)
    add_subdirectory(Brimstone)
endif()

# Installation rules
install(TARGETS RtModel Graph
    EXPORT BrimstoneTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(BUILD_BRIMSTONE_APP)
    install(TARGETS Brimstone
        RUNTIME DESTINATION bin
    )
endif()

# Export targets for use by other projects
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
