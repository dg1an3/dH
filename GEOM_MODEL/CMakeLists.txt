# GEOM_MODEL library

add_library(GEOM_MODEL STATIC
    AffineTransform.cpp
    Cluster.cpp
    Dib.cpp
    GradientCalculator.cpp
    Mesh.cpp
    ModelObject.cpp
    Observer.cpp
    Polygon.cpp
    Pyramid.cpp
    Subsampler.cpp
    TPSTransform.cpp
    StdAfx.cpp
)

target_include_directories(GEOM_MODEL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GEOM_MODEL
    PUBLIC MTL
    PUBLIC XMLLogging
)

target_compile_definitions(GEOM_MODEL PRIVATE
    USE_XMLLOGGING
    $<$<CONFIG:Debug>:XMLLOGGING_ON>
)

# Optional: USE_IPP if Intel Performance Primitives are available
# target_compile_definitions(GEOM_MODEL PRIVATE USE_IPP)

# Precompiled header support
# target_precompile_headers(GEOM_MODEL PRIVATE StdAfx.h)
