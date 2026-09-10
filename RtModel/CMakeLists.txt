# RtModel: core optimization, dose calculation and data models (static lib).
# Source list mirrors RtModel.vcxproj. SigmaEstimator*.cpp and
# TG263Nomenclature.cpp exist in the tree but are not in the vcxproj, so they
# are left out here too for parity.

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
    include/SigmoidParams.h
    include/SphereConvolve.h
    include/Structure.h
    include/utilmacros.h
    include/VectorN.h
    include/VectorOps.h
    include/VOITerm.h
    stdafx.h
)

add_library(RtModel STATIC ${RTMODEL_SOURCES} ${RTMODEL_HEADERS})

target_include_directories(RtModel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DH_VXL_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ITK and VNL are part of RtModel's public interface (VolumeReal, vnl_vector
# etc. appear in the headers).
target_link_libraries(RtModel PUBLIC ${ITK_LIBRARIES} vnl_algo vnl vcl v3p_netlib)

target_compile_definitions(RtModel PRIVATE _LIB)

if(DH_IPP_FOUND)
    # USE_IPP is PUBLIC: VectorOps.h / ItkUtils.h switch on it and are
    # included by Graph and Brimstone, whose vcxprojs define it as well.
    target_link_libraries(RtModel PUBLIC IPP::IPP)
    target_compile_definitions(RtModel PUBLIC USE_IPP)
endif()

target_precompile_headers(RtModel PRIVATE stdafx.h)
