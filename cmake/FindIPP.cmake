# FindIPP.cmake
# ---------------
# Find Intel Integrated Performance Primitives (IPP)
#
# This module will first look for IPP installed via NuGet package,
# then fall back to system-installed IPP (oneAPI or standalone).
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   IPP_FOUND          - True if IPP is found
#   IPP_INCLUDE_DIRS   - Include directories for IPP
#   IPP_LIBRARIES      - Libraries to link against
#   IPP_VERSION        - Version of IPP found
#
# Components
# ^^^^^^^^^^
#
# The following components are supported:
#   ippi    - Image processing
#   ipps    - Signal processing
#   ippm    - Matrix operations
#   ippcore - Core functionality

# Try NuGet package first (Windows builds)
if(WIN32)
    set(IPP_NUGET_PACKAGE_NAME "intelipp.devel.win-x64")

    # Check common NuGet package locations
    set(NUGET_SEARCH_PATHS
        "${CMAKE_SOURCE_DIR}/packages"
        "${CMAKE_SOURCE_DIR}/../packages"
        "${CMAKE_BINARY_DIR}/packages"
    )

    foreach(SEARCH_PATH ${NUGET_SEARCH_PATHS})
        file(GLOB IPP_NUGET_DIRS "${SEARCH_PATH}/${IPP_NUGET_PACKAGE_NAME}*")
        if(IPP_NUGET_DIRS)
            list(GET IPP_NUGET_DIRS 0 IPP_NUGET_ROOT)
            break()
        endif()
    endforeach()

    if(IPP_NUGET_ROOT)
        message(STATUS "Found IPP NuGet package at: ${IPP_NUGET_ROOT}")

        # Extract version from directory name
        get_filename_component(IPP_DIR_NAME ${IPP_NUGET_ROOT} NAME)
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" IPP_VERSION ${IPP_DIR_NAME})

        # Set include and library paths
        set(IPP_INCLUDE_DIRS "${IPP_NUGET_ROOT}/build/native/include")

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(IPP_LIB_DIR "${IPP_NUGET_ROOT}/build/native/win-x64")
            set(IPP_BIN_DIR "${IPP_NUGET_ROOT}/runtimes/win-x64/native")
        else()
            set(IPP_LIB_DIR "${IPP_NUGET_ROOT}/build/native/win-x86")
            set(IPP_BIN_DIR "${IPP_NUGET_ROOT}/runtimes/win-x86/native")
        endif()

        # Find requested component libraries
        set(IPP_LIBRARIES "")
        set(IPP_COMPONENT_FOUND TRUE)

        if(IPP_FIND_COMPONENTS)
            set(COMPONENTS ${IPP_FIND_COMPONENTS})
        else()
            # Default components if none specified
            set(COMPONENTS ippi ipps ippm ippcore)
        endif()

        foreach(COMPONENT ${COMPONENTS})
            find_library(IPP_${COMPONENT}_LIBRARY
                NAMES ${COMPONENT}
                PATHS ${IPP_LIB_DIR}
                NO_DEFAULT_PATH
            )

            if(IPP_${COMPONENT}_LIBRARY)
                list(APPEND IPP_LIBRARIES ${IPP_${COMPONENT}_LIBRARY})
                set(IPP_${COMPONENT}_FOUND TRUE)
            else()
                message(WARNING "IPP component ${COMPONENT} not found")
                set(IPP_COMPONENT_FOUND FALSE)
            endif()
        endforeach()

        if(IPP_COMPONENT_FOUND)
            set(IPP_FOUND TRUE)
        endif()
    endif()
endif()

# Fall back to system-installed IPP (oneAPI or standalone)
if(NOT IPP_FOUND)
    # Try to find IPP via IPPROOT environment variable or standard paths
    find_path(IPP_INCLUDE_DIR
        NAMES ipp.h
        PATHS
            ENV IPPROOT
            ENV IPP_ROOT
            ENV ONEAPI_ROOT
        PATH_SUFFIXES
            include
            ipp/include
    )

    if(IPP_INCLUDE_DIR)
        set(IPP_INCLUDE_DIRS ${IPP_INCLUDE_DIR})

        # Determine library path
        get_filename_component(IPP_ROOT_DIR ${IPP_INCLUDE_DIR} DIRECTORY)

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(IPP_LIB_PATHS "${IPP_ROOT_DIR}/lib/intel64" "${IPP_ROOT_DIR}/lib")
        else()
            set(IPP_LIB_PATHS "${IPP_ROOT_DIR}/lib/ia32" "${IPP_ROOT_DIR}/lib")
        endif()

        # Find requested component libraries
        set(IPP_LIBRARIES "")
        set(IPP_COMPONENT_FOUND TRUE)

        if(IPP_FIND_COMPONENTS)
            set(COMPONENTS ${IPP_FIND_COMPONENTS})
        else()
            set(COMPONENTS ippi ipps ippm ippcore)
        endif()

        foreach(COMPONENT ${COMPONENTS})
            find_library(IPP_${COMPONENT}_LIBRARY
                NAMES ${COMPONENT}
                PATHS ${IPP_LIB_PATHS}
                NO_DEFAULT_PATH
            )

            if(IPP_${COMPONENT}_LIBRARY)
                list(APPEND IPP_LIBRARIES ${IPP_${COMPONENT}_LIBRARY})
                set(IPP_${COMPONENT}_FOUND TRUE)
            else()
                message(WARNING "IPP component ${COMPONENT} not found")
                set(IPP_COMPONENT_FOUND FALSE)
            endif()
        endforeach()

        if(IPP_COMPONENT_FOUND)
            set(IPP_FOUND TRUE)
        endif()
    endif()
endif()

# Handle standard arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IPP
    REQUIRED_VARS IPP_LIBRARIES IPP_INCLUDE_DIRS
    VERSION_VAR IPP_VERSION
    HANDLE_COMPONENTS
)

# Create imported target
if(IPP_FOUND AND NOT TARGET IPP::IPP)
    add_library(IPP::IPP INTERFACE IMPORTED)
    set_target_properties(IPP::IPP PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${IPP_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${IPP_LIBRARIES}"
    )
endif()

mark_as_advanced(IPP_INCLUDE_DIRS IPP_LIBRARIES)
