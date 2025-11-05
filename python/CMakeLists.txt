cmake_minimum_required(VERSION 3.12)
project(rtmodel_python)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python and pybind11
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Find VNL (part of VXL)
find_package(VXL REQUIRED)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

# Find ITK (optional, for full integration)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Include directories for RtModel
include_directories(
    ${CMAKE_SOURCE_DIR}/../RtModel/include
    ${CMAKE_SOURCE_DIR}/../RtModel
    ${VNL_INCLUDE_DIRS}
)

# Collect RtModel source files
file(GLOB RTMODEL_SOURCES
    ../RtModel/*.cpp
)

# Remove Windows-specific files if on Linux
if(UNIX)
    list(FILTER RTMODEL_SOURCES EXCLUDE REGEX ".*stdafx.cpp$")
endif()

# Create the Python module
pybind11_add_module(rtmodel_core
    rtmodel_bindings.cpp
    ${RTMODEL_SOURCES}
)

# Link libraries
target_link_libraries(rtmodel_core PRIVATE
    ${VNL_LIBRARIES}
    ${ITK_LIBRARIES}
)

# Set compile definitions
target_compile_definitions(rtmodel_core PRIVATE
    _USE_MATH_DEFINES
)

# Install the module
install(TARGETS rtmodel_core DESTINATION .)
