# PenBeamEdit - Pencil beam editor application (wxWidgets version)

# wxWidgets application executable
add_executable(PenBeamEdit WIN32
    PenBeamEditApp.cpp
    MainFrame.cpp
    PenBeamEditCanvas.cpp
)

# NOTE: Old MFC files retained for reference:
# - PenBeamEdit.cpp/h (MFC version)
# - MainFrm.cpp/h (MFC version)
# - PenBeamEditView.cpp/h (MFC version)
# - StdAfx.cpp/h (MFC-specific)
# - PenBeamEdit.rc (MFC resources)

target_include_directories(PenBeamEdit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with wxWidgets
target_link_libraries(PenBeamEdit PRIVATE
    ${wxWidgets_LIBRARIES}
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

# Set subsystem to Windows (GUI application)
set_target_properties(PenBeamEdit PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Note: The dependent libraries still use MFC. A full migration would require
# refactoring RT_MODEL, GEOM_VIEW, and other libraries to remove MFC dependencies
# (CString, CArray, CObject, serialization, etc.)
