cmake_minimum_required(VERSION 3.15)
project(dH_Project VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# MFC settings - required for library projects
# Note: PenBeamEdit uses wxWidgets instead of MFC
set(CMAKE_MFC_FLAG 2)  # Use MFC in a shared DLL for libraries

# wxWidgets setup for PenBeamEdit
find_package(wxWidgets COMPONENTS core base REQUIRED)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
endif()

# Global compile definitions
if(WIN32)
    add_compile_definitions(
        WIN32
        _WINDOWS
        _MBCS
    )
endif()

# Build configuration specific definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
else()
    add_compile_definitions(NDEBUG)
endif()

# Add subdirectories in dependency order
# Base libraries (no dependencies)
add_subdirectory(MTL)
add_subdirectory(XMLLogging)
add_subdirectory(MODEL_BASE)

# Geometry base (depends on MODEL_BASE)
add_subdirectory(GEOM_BASE)

# GUI and optimizer bases
add_subdirectory(GUI_BASE)
add_subdirectory(OPTIMIZER_BASE)

# Model libraries
add_subdirectory(GEOM_MODEL)
add_subdirectory(GEOM_VIEW)
add_subdirectory(RT_MODEL)

# Executable applications
add_subdirectory(PenBeamEdit)
