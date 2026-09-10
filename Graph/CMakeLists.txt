# Graph: DVH / iteration graph widgets (static lib). Mirrors Graph.vcxproj.

set(GRAPH_SOURCES
    DataSeries.cpp
    Dib.cpp
    Graph.cpp
    HistogramDataSeries.cpp
    TargetDVHSeries.cpp
    stdafx.cpp
)

set(GRAPH_HEADERS
    include/DataSeries.h
    include/Dib.h
    include/Graph.h
    include/HistogramDataSeries.h
    include/TargetDVHSeries.h
    stdafx.h
)

add_library(Graph STATIC ${GRAPH_SOURCES} ${GRAPH_HEADERS})

target_include_directories(Graph
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Graph PUBLIC RtModel)

target_compile_definitions(Graph PRIVATE _LIB)

target_precompile_headers(Graph PRIVATE stdafx.h)
