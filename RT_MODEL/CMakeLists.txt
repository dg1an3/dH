# RT_MODEL library - Radiotherapy model

add_library(RT_MODEL STATIC
    BasisGroup.cpp
    Beam.cpp
    BeamDoseCalc.cpp
    EnergyDepKernel.cpp
    Histogram.cpp
    KLDivTerm.cpp
    Plan.cpp
    Prescription.cpp
    Series.cpp
    Structure.cpp
    TCP_NTCP_Optimizer.cpp
    TreatmentMachine.cpp
    VOITerm.cpp
    StdAfx.cpp
)

target_include_directories(RT_MODEL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RT_MODEL
    PUBLIC GEOM_MODEL
    PUBLIC GEOM_VIEW
    PUBLIC OPTIMIZER_BASE
    PUBLIC MTL
    PUBLIC XMLLogging
)

target_compile_definitions(RT_MODEL PUBLIC
    _AFXDLL
    USE_XMLLOGGING
    $<$<CONFIG:Debug>:XMLLOGGING_ON>
)

# Precompiled header support
# target_precompile_headers(RT_MODEL PRIVATE StdAfx.h)
