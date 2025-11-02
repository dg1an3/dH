# MTL - Mathematical Template Library
# Header-only library with minimal implementation

add_library(MTL INTERFACE)

target_include_directories(MTL INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Note: MTL appears to be mostly header-only
# If there are .cpp files that need to be compiled, they should be added here
