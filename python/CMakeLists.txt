# rtmodel_core: pybind11 bindings for the RtModel optimizer.
#
# Built as part of the root CMake project (option DH_BUILD_PYTHON) and linked
# against the RtModel target, so the ITK/VNL/IPP/MFC settings come from that
# target's usage requirements instead of being restated here (previously this
# file re-globbed RtModel/*.cpp and setup.py carried a third copy of the link
# line). Configure and build from the repo root, e.g.
#
#   cmake --preset vs2022-x64 -DDH_BUILD_PYTHON=ON
#   cmake --build --preset vs2022-x64-release --target rtmodel_core
#
# The module lands in build/<preset>/bin/<Config>/rtmodel_core.*.pyd next to
# the vcpkg / IPP DLLs it needs (vcpkg applocal deploys them for the .pyd).
#
# pybind11 comes from the Python interpreter's pip package (pybind11 >= 2.10,
# `python -m pybind11 --cmakedir`); a vcpkg "pybind11" port also works if
# pybind11_DIR is set. Pass -DPython_EXECUTABLE=<python.exe> to pick the
# interpreter (it must be the x64 build the module will be imported from).

if(NOT TARGET RtModel)
    message(FATAL_ERROR "python/CMakeLists.txt must be added from the root project (RtModel target missing)")
endif()

find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

if(NOT pybind11_DIR AND NOT pybind11_FOUND)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE _pybind11_cmakedir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _pybind11_rc
    )
    if(_pybind11_rc EQUAL 0 AND EXISTS "${_pybind11_cmakedir}")
        set(pybind11_DIR "${_pybind11_cmakedir}" CACHE PATH "pybind11 CMake config dir (from pip)")
    endif()
endif()
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(rtmodel_core MODULE rtmodel_bindings.cpp)

# RtModel's PUBLIC usage requirements supply: RtModel/include, ITK, VNL/VXL,
# IPP (+USE_IPP), and the tree-wide definitions (UNICODE/_UNICODE, NOMINMAX,
# USE_RTOPT, WIN32) come from the root. CMAKE_MFC_FLAG=2 at the root gives
# _AFXDLL and the MFC pragma-links.
target_link_libraries(rtmodel_core PRIVATE RtModel)

target_compile_definitions(rtmodel_core PRIVATE
    _WINDOWS
    VERSION_INFO="${PROJECT_VERSION}"
)

# /bigobj: MFC + ITK template instantiations exceed 64k sections in one TU.
# /wd4996: CRT/STL deprecation noise from the older RtModel code.
target_compile_options(rtmodel_core PRIVATE /bigobj /wd4996)

# Keep the module next to Brimstone.exe so both share one deployed DLL set.
set_target_properties(rtmodel_core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# IPP runtime DLLs next to the module (Brimstone's post-build does the same).
if(DH_IPP_FOUND AND IPP_BIN_DIR)
    file(GLOB _dh_py_ipp_dlls "${IPP_BIN_DIR}/ipps*.dll" "${IPP_BIN_DIR}/ippcore*.dll")
    if(_dh_py_ipp_dlls)
        add_custom_command(TARGET rtmodel_core POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_dh_py_ipp_dlls} $<TARGET_FILE_DIR:rtmodel_core>
            COMMENT "rtmodel_core: deploying IPP runtime DLLs"
            VERBATIM
        )
    endif()
endif()

# Smoke test: import the module from its build directory and check the
# bound classes are there. Python 3.8+ resolves extension-module DLLs with a
# restricted search and IPP's ipps.dll cannot find ippcore.dll through it, so
# consumers go through pybrimstone.native.import_rtmodel_core(), which
# registers the directory and pre-loads the IPP DLLs. The test does the same.
add_test(NAME rtmodel_core_import
    COMMAND "${Python_EXECUTABLE}" -c
        "import os, sys; sys.path.insert(0, r'${CMAKE_CURRENT_SOURCE_DIR}'); from pybrimstone.native import import_rtmodel_core; m = import_rtmodel_core(os.getcwd()); names = [n for n in dir(m) if not n.startswith('_')]; print(m.__file__, names); assert 'ConjGradOptimizer' in names and 'PrescriptionWrapper' in names"
    WORKING_DIRECTORY $<TARGET_FILE_DIR:rtmodel_core>
)
