# PenBeamEdit - Pencil beam editor application

add_executable(PenBeamEdit WIN32
    MainFrm.cpp
    PenBeamEdit.cpp
    PenBeamEditView.cpp
    StdAfx.cpp
    PenBeamEdit.rc
)

target_include_directories(PenBeamEdit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PenBeamEdit PRIVATE
    RT_MODEL
    GEOM_VIEW
    GEOM_MODEL
    GEOM_BASE
    GUI_BASE
    MODEL_BASE
    OPTIMIZER_BASE
    XMLLogging
    MTL
)

target_compile_definitions(PenBeamEdit PRIVATE
    USE_XMLLOGGING
    XMLLOGGING_ON
)

# Precompiled header support
# target_precompile_headers(PenBeamEdit PRIVATE StdAfx.h)

# Set subsystem to Windows (GUI application)
set_target_properties(PenBeamEdit PROPERTIES
    WIN32_EXECUTABLE TRUE
)
