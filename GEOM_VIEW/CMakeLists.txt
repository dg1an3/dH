# GEOM_VIEW library

add_library(GEOM_VIEW STATIC
    Camera.cpp
    DibView.cpp
    Graph.cpp
    Light.cpp
    MarchingSquares.cpp
    ObjectExplorer.cpp
    ObjectTreeItem.cpp
    RenderContext.cpp
    Renderable.cpp
    RotateTracker.cpp
    SceneView.cpp
    SurfaceRenderable.cpp
    TabControlBar.cpp
    Texture.cpp
    Tracker.cpp
    ZoomTracker.cpp
    StdAfx.cpp
)

target_include_directories(GEOM_VIEW PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GEOM_VIEW
    PUBLIC GEOM_MODEL
    PUBLIC MTL
    PUBLIC XMLLogging
)

target_compile_definitions(GEOM_VIEW PRIVATE
    USE_XMLLOGGING
)

# Resource file (if needed)
# target_sources(GEOM_VIEW PRIVATE GUI_BASE.rc)

# Link OpenGL libraries (required for rendering)
target_link_libraries(GEOM_VIEW
    PUBLIC opengl32
)

# Precompiled header support
# target_precompile_headers(GEOM_VIEW PRIVATE StdAfx.h)
