# GEOM_BASE library

add_library(GEOM_BASE STATIC
    Polygon.cpp
    StdAfx.cpp
)

target_include_directories(GEOM_BASE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GEOM_BASE
    PUBLIC MODEL_BASE
)

# Precompiled header support
# target_precompile_headers(GEOM_BASE PRIVATE StdAfx.h)
