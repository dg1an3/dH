cmake_minimum_required(VERSION 3.16)

project(RtModel)

# Collect source files
set(RTMODEL_SOURCES
    Beam.cpp
    BeamDoseCalc.cpp
    ConjGradOptimizer.cpp
    EnergyDepKernel.cpp
    Histogram.cpp
    HistogramGradient.cpp
    KLDivTerm.cpp
    ObjectiveFunction.cpp
    Plan.cpp
    PlanOptimizer.cpp
    PlanPyramid.cpp
    PlanXmlFile.cpp
    Prescription.cpp
    Series.cpp
    SphereConvolve.cpp
    Structure.cpp
    VOITerm.cpp
    stdafx.cpp
)

# Collect header files
set(RTMODEL_HEADERS
    include/Beam.h
    include/BeamDoseCalc.h
    include/ConjGradOptimizer.h
    include/EnergyDepKernel.h
    include/Histogram.h
    include/HistogramGradient.h
    include/ItkUtils.h
    include/KLDivTerm.h
    include/MathUtil.h
    include/MatrixNxM.h
    include/ObjectiveFunction.h
    include/Plan.h
    include/PlanOptimizer.h
    include/PlanPyramid.h
    include/PlanXmlFile.h
    include/Prescription.h
    include/Series.h
    include/SphereConvolve.h
    include/Structure.h
    include/VOITerm.h
    stdafx.h
)

# Create the library
add_library(RtModel STATIC ${RTMODEL_SOURCES} ${RTMODEL_HEADERS})

# Set target properties
target_include_directories(RtModel
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link ITK libraries
target_link_libraries(RtModel
    PUBLIC
        ${ITK_LIBRARIES}
)

# Compile definitions for Windows
if(MSVC)
    target_compile_definitions(RtModel PRIVATE
        UNICODE
        _UNICODE
        _AFXDLL
        WINVER=0x0501
        _WIN32_WINNT=0x0501
    )
    
    # Use precompiled headers
    target_precompile_headers(RtModel PRIVATE stdafx.h)
    
    # Set MFC usage
    set_target_properties(RtModel PROPERTIES
        COMPILE_FLAGS "/D_AFXDLL"
    )
endif()

# Installation
install(TARGETS RtModel
    EXPORT RtModelTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/RtModel
    FILES_MATCHING PATTERN "*.h"
)
