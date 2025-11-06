cmake_minimum_required(VERSION 3.16)

project(Brimstone)

# Collect source files
set(BRIMSTONE_SOURCES
    Brimstone.cpp
    BrimstoneDoc.cpp
    BrimstoneView.cpp
    MainFrm.cpp
    OptThread.cpp
    PlanSetupDlg.cpp
    PlanarView.cpp
    PrescriptionBar.cpp
    PrescriptionToolbar.cpp
    SeriesDicomImporter.cpp
    stdafx.cpp
)

# Collect header files
set(BRIMSTONE_HEADERS
    Brimstone.h
    BrimstoneDoc.h
    BrimstoneView.h
    MainFrm.h
    OptThread.h
    PlanSetupDlg.h
    PlanarView.h
    PrescriptionBar.h
    PrescriptionToolbar.h
    SeriesDicomImporter.h
    Resource.h
    StdAfx.h
)

# Resource files
set(BRIMSTONE_RESOURCES
    Brimstone.rc
    res/Brimstone.rc2
)

# Create the executable
add_executable(Brimstone WIN32
    ${BRIMSTONE_SOURCES}
    ${BRIMSTONE_HEADERS}
    ${BRIMSTONE_RESOURCES}
)

# Set target properties
target_include_directories(Brimstone
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/RtModel/include
        ${CMAKE_SOURCE_DIR}/Graph/include
)

# Link dependencies
target_link_libraries(Brimstone
    PRIVATE
        RtModel
        Graph
)

# Link Intel IPP libraries if available (via RtModel, but also directly for PlanarView.cpp)
if(IPP_FOUND)
    target_link_libraries(Brimstone
        PRIVATE
            IPP::IPP
    )
    message(STATUS "Brimstone: Intel IPP optimizations enabled")
else()
    message(STATUS "Brimstone: Building without Intel IPP optimizations")
endif()

# Compile definitions for Windows MFC application
if(MSVC)
    target_compile_definitions(Brimstone PRIVATE
        UNICODE
        _UNICODE
        _AFXDLL
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )

    # Workaround for VNL SSE inline issues with modern MSVC
    target_compile_options(Brimstone PRIVATE /DVNL_SSE_FORCE_INLINE=inline)

    # Use precompiled headers
    # Temporarily disabled due to VNL SSE inline issues
    # target_precompile_headers(Brimstone PRIVATE StdAfx.h)
    
    # Set MFC usage
    set_target_properties(Brimstone PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

# Copy kernel data files to output directory
file(GLOB KERNEL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.dat")
foreach(KERNEL_FILE ${KERNEL_FILES})
    configure_file(${KERNEL_FILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/ COPYONLY)
endforeach()

# Installation
install(TARGETS Brimstone
    RUNTIME DESTINATION bin
)

# Install kernel data files
install(FILES ${KERNEL_FILES}
    DESTINATION bin
)
