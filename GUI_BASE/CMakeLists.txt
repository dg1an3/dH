# GUI_BASE library

add_library(GUI_BASE STATIC
    Dib.cpp
    Graph.cpp
    ObjectExplorer.cpp
    ObjectExplorerControlBar.cpp
    ObjectTreeItem.cpp
    TabControlBar.cpp
    StdAfx.cpp
)

target_include_directories(GUI_BASE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GUI_BASE
    PUBLIC GEOM_BASE
    PUBLIC MODEL_BASE
)

# Resource file (if needed)
# target_sources(GUI_BASE PRIVATE GUI_BASE.rc)

# Precompiled header support
# target_precompile_headers(GUI_BASE PRIVATE StdAfx.h)
