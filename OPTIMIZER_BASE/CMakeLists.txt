# OPTIMIZER_BASE library

add_library(OPTIMIZER_BASE STATIC
    BrentOptimizer.cpp
    StdAfx.cpp
)

target_include_directories(OPTIMIZER_BASE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OPTIMIZER_BASE
    PUBLIC GEOM_MODEL
)

# Precompiled header support
# target_precompile_headers(OPTIMIZER_BASE PRIVATE StdAfx.h)
