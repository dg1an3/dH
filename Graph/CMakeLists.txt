cmake_minimum_required(VERSION 3.16)

project(Graph)

# Collect source files
set(GRAPH_SOURCES
    DataSeries.cpp
    Dib.cpp
    Graph.cpp
    HistogramDataSeries.cpp
    TargetDVHSeries.cpp
    stdafx.cpp
)

# Collect header files
set(GRAPH_HEADERS
    include/DataSeries.h
    include/Dib.h
    include/Graph.h
    include/HistogramDataSeries.h
    include/TargetDVHSeries.h
    stdafx.h
)

# Create the library
add_library(Graph STATIC ${GRAPH_SOURCES} ${GRAPH_HEADERS})

# Set target properties
target_include_directories(Graph
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/RtModel/include
        ${CMAKE_SOURCE_DIR}/GEOM_MODEL/include
)

# Link dependencies
target_link_libraries(Graph
    PUBLIC
        RtModel
)

# Link Intel IPP libraries if available (needed for VectorOps.h in RtModel headers)
if(IPP_FOUND)
    target_link_libraries(Graph
        PUBLIC
            IPP::IPP
    )
    message(STATUS "Graph: Intel IPP optimizations enabled")
endif()

# Compile definitions for Windows
if(MSVC)
    target_compile_definitions(Graph PUBLIC
        USE_RTOPT
    )
    target_compile_definitions(Graph PRIVATE
        UNICODE
        _UNICODE
        _AFXDLL
        NOMINMAX
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )

    # Disable VNL SSE optimizations and prevent Windows min/max macros
    target_compile_definitions(Graph PRIVATE
        VNL_CONFIG_ENABLE_SSE2=0
        VNL_CONFIG_ENABLE_SSE2_ROUNDING=0
        NOMINMAX
    )

    # Use precompiled headers
    # Temporarily disabled due to VNL SSE inline issues
    # target_precompile_headers(Graph PRIVATE stdafx.h)
    
    # Set MFC usage
    set_target_properties(Graph PROPERTIES
        COMPILE_FLAGS "/D_AFXDLL"
    )
endif()

# Installation
install(TARGETS Graph
    EXPORT GraphTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/Graph
    FILES_MATCHING PATTERN "*.h"
)
