# CMakeLists.txt for RtModel and RT_VIEW tests
cmake_minimum_required(VERSION 3.10)
project(RtModel_Tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/catch2
    ${CMAKE_CURRENT_SOURCE_DIR}/../RtModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../RT_VIEW/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../GEOM_MODEL/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../OPTIMIZER_BASE/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../MTL/include
)

# Find ITK (required by RtModel)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# RtModel library tests
add_executable(test_rtmodel_plan
    RtModel/test_plan.cpp
)

add_executable(test_rtmodel_histogram
    RtModel/test_histogram.cpp
)

add_executable(test_rtmodel_optimizer
    RtModel/test_optimizer.cpp
)

add_executable(test_rtmodel_dose_calc
    RtModel/test_dose_calc.cpp
)

# RT_VIEW library tests
add_executable(test_rtview_beam
    RT_VIEW/test_beam_renderable.cpp
)

# Link libraries
target_link_libraries(test_rtmodel_plan
    RtModel
    ${ITK_LIBRARIES}
)

target_link_libraries(test_rtmodel_histogram
    RtModel
    ${ITK_LIBRARIES}
)

target_link_libraries(test_rtmodel_optimizer
    RtModel
    ${ITK_LIBRARIES}
)

target_link_libraries(test_rtmodel_dose_calc
    RtModel
    ${ITK_LIBRARIES}
)

target_link_libraries(test_rtview_beam
    RT_VIEW
    RtModel
    ${ITK_LIBRARIES}
)

# Enable testing
enable_testing()

# Add tests to CTest
add_test(NAME RtModel_Plan COMMAND test_rtmodel_plan)
add_test(NAME RtModel_Histogram COMMAND test_rtmodel_histogram)
add_test(NAME RtModel_Optimizer COMMAND test_rtmodel_optimizer)
add_test(NAME RtModel_DoseCalc COMMAND test_rtmodel_dose_calc)
add_test(NAME RT_VIEW_Beam COMMAND test_rtview_beam)

# Optional: Create a combined test executable
add_executable(run_all_tests
    RtModel/test_plan.cpp
    RtModel/test_histogram.cpp
    RtModel/test_optimizer.cpp
    RtModel/test_dose_calc.cpp
    RT_VIEW/test_beam_renderable.cpp
)

target_link_libraries(run_all_tests
    RtModel
    RT_VIEW
    ${ITK_LIBRARIES}
)

add_test(NAME AllTests COMMAND run_all_tests)

# Set test properties
set_tests_properties(RtModel_Plan PROPERTIES
    TIMEOUT 30
    LABELS "RtModel;Core"
)

set_tests_properties(RtModel_Histogram PROPERTIES
    TIMEOUT 60
    LABELS "RtModel;Histogram"
)

set_tests_properties(RtModel_Optimizer PROPERTIES
    TIMEOUT 120
    LABELS "RtModel;Optimizer"
)

set_tests_properties(RtModel_DoseCalc PROPERTIES
    TIMEOUT 120
    LABELS "RtModel;DoseCalc"
)

set_tests_properties(RT_VIEW_Beam PROPERTIES
    TIMEOUT 30
    LABELS "RT_VIEW;Rendering"
)

# Installation
install(TARGETS
    test_rtmodel_plan
    test_rtmodel_histogram
    test_rtmodel_optimizer
    test_rtmodel_dose_calc
    test_rtview_beam
    run_all_tests
    RUNTIME DESTINATION bin/tests
)

# Custom target for running Prolog tests (requires SWI-Prolog)
add_custom_target(test_prolog
    COMMAND swipl -g "run_all_tests,halt" -t "halt(1)" ${CMAKE_CURRENT_SOURCE_DIR}/prolog/rtmodel_verification.pl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/prolog
    COMMENT "Running Prolog verification tests..."
)

# Custom target for running all tests (C++ and Prolog)
add_custom_target(test_all
    DEPENDS test_prolog
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    COMMENT "Running all tests (C++ and Prolog)..."
)
