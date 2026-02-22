cmake_minimum_required(VERSION 3.16)

project(Brimstone)

# Try to find DCMTK (optional - needed for DICOM import)
find_package(DCMTK QUIET)

# Collect source files
# Note: PrescriptionBar.cpp is C++/CLI managed code and excluded from native build
set(BRIMSTONE_SOURCES
    Brimstone.cpp
    BrimstoneDoc.cpp
    BrimstoneView.cpp
    MainFrm.cpp
    OptThread.cpp
    PlanSetupDlg.cpp
    PlanarView.cpp
    PrescriptionToolbar.cpp
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
    PrescriptionToolbar.h
    Resource.h
    StdAfx.h
)

# DICOM importer requires DCMTK
if(DCMTK_FOUND)
    list(APPEND BRIMSTONE_SOURCES SeriesDicomImporter.cpp)
    list(APPEND BRIMSTONE_HEADERS SeriesDicomImporter.h)
    message(STATUS "Brimstone: DCMTK found, DICOM import enabled")
else()
    message(STATUS "Brimstone: DCMTK not found, DICOM import disabled")
endif()

# Add HAS_DCMTK define if DCMTK was found (set after target creation below)

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
        # Legacy library includes
        ${CMAKE_SOURCE_DIR}/XMLLogging
        ${CMAKE_SOURCE_DIR}/GEOM_BASE
        ${CMAKE_SOURCE_DIR}/MODEL_BASE
        ${CMAKE_SOURCE_DIR}/MTL
        ${CMAKE_SOURCE_DIR}/VecMat
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
        NOMINMAX
        USE_RTOPT
        WINVER=0x0501
        _WIN32_WINNT=0x0501
        $<$<BOOL:${DCMTK_FOUND}>:HAS_DCMTK>
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
