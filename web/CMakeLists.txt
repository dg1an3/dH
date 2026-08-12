cmake_minimum_required(VERSION 3.20)
project(BrimstoneWeb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find vcpkg packages
find_package(nlohmann_json CONFIG REQUIRED)

# ITK configuration (use existing dH ITK setup)
set(ITK_DIR "$ENV{ITK_DIR}" CACHE PATH "Path to ITK build directory")
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Add subdirectories
add_subdirectory(service)
