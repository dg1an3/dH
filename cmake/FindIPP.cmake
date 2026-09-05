# FindIPP.cmake
# -------------
# Locate Intel Integrated Performance Primitives (oneAPI IPP).
#
# Looks, in order, at IPP_ROOT (cmake variable or environment), IPPROOT /
# ONEAPI_ROOT environment variables, and the default oneAPI install used by
# Brimstone.vcxproj ("C:/Program Files (x86)/Intel/oneAPI/ipp/latest").
#
# Components (default: ipps ippcore -- what Brimstone.vcxproj links):
#   ipps ippcore ippi ippcv ippvm ...  any library name under <root>/lib
#
# Result variables / targets:
#   IPP_FOUND, IPP_INCLUDE_DIRS, IPP_LIBRARIES, IPP_ROOT_DIR, IPP_BIN_DIR
#   IPP_<component>_LIBRARY, IPP_<component>_FOUND
#   IPP::IPP  imported interface target (includes + requested libraries)

set(_ipp_hints "")
if(IPP_ROOT)
    list(APPEND _ipp_hints "${IPP_ROOT}")
endif()
foreach(_env IPP_ROOT IPPROOT)
    if(DEFINED ENV{${_env}})
        list(APPEND _ipp_hints "$ENV{${_env}}")
    endif()
endforeach()
if(DEFINED ENV{ONEAPI_ROOT})
    list(APPEND _ipp_hints "$ENV{ONEAPI_ROOT}/ipp/latest")
endif()
list(APPEND _ipp_hints
    "C:/Program Files (x86)/Intel/oneAPI/ipp/latest"
    "C:/Program Files/Intel/oneAPI/ipp/latest"
    "/opt/intel/oneapi/ipp/latest"
)

# ipp.h sits directly in <root>/include (a copy also lives in include/ipp).
find_path(IPP_INCLUDE_DIR
    NAMES ipp.h
    HINTS ${_ipp_hints}
    PATH_SUFFIXES include
)

if(IPP_INCLUDE_DIR)
    get_filename_component(IPP_ROOT_DIR "${IPP_INCLUDE_DIR}" DIRECTORY)
    set(IPP_INCLUDE_DIRS "${IPP_INCLUDE_DIR}" "${IPP_INCLUDE_DIR}/ipp")
    set(IPP_BIN_DIR "${IPP_ROOT_DIR}/bin")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_ipp_lib_dirs "${IPP_ROOT_DIR}/lib" "${IPP_ROOT_DIR}/lib/intel64")
    else()
        set(_ipp_lib_dirs "${IPP_ROOT_DIR}/lib/ia32" "${IPP_ROOT_DIR}/lib32")
    endif()

    if(IPP_FIND_COMPONENTS)
        set(_ipp_components ${IPP_FIND_COMPONENTS})
    else()
        set(_ipp_components ipps ippcore)
    endif()

    set(IPP_LIBRARIES "")
    foreach(_c ${_ipp_components})
        find_library(IPP_${_c}_LIBRARY
            NAMES ${_c}
            PATHS ${_ipp_lib_dirs}
            NO_DEFAULT_PATH
        )
        if(IPP_${_c}_LIBRARY)
            set(IPP_${_c}_FOUND TRUE)
            list(APPEND IPP_LIBRARIES "${IPP_${_c}_LIBRARY}")
        else()
            set(IPP_${_c}_FOUND FALSE)
        endif()
        mark_as_advanced(IPP_${_c}_LIBRARY)
    endforeach()

    # version from ippversion.h if present
    if(EXISTS "${IPP_INCLUDE_DIR}/ippversion.h")
        file(STRINGS "${IPP_INCLUDE_DIR}/ippversion.h" _ipp_ver_line
             REGEX "#define[ \t]+IPP_VERSION_STR[ \t]+\"")
        if(_ipp_ver_line)
            string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1" IPP_VERSION "${_ipp_ver_line}")
        endif()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IPP
    REQUIRED_VARS IPP_INCLUDE_DIR IPP_LIBRARIES
    VERSION_VAR IPP_VERSION
    HANDLE_COMPONENTS
)

if(IPP_FOUND AND NOT TARGET IPP::IPP)
    add_library(IPP::IPP INTERFACE IMPORTED)
    set_target_properties(IPP::IPP PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${IPP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${IPP_LIBRARIES}"
    )
endif()

mark_as_advanced(IPP_INCLUDE_DIR)
