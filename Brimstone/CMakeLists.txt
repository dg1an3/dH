# Brimstone: MFC SDI application. Mirrors Brimstone.vcxproj (Release|x64):
# DICOM import via DCMTK, WebView2-hosted DVH / convergence / planar views,
# IPP-accelerated planar view, kernel .dat + webassets deployed next to the exe.
#
# OptimizerDashboard.cpp and PrescDlg.cpp exist in the tree but are not in
# the vcxproj and are left out for parity. PrescriptionBar.cpp/.h is a
# C++/CLI (System::Windows::Forms) relic that the vcxproj compiles with
# CompileAsManaged only in the Win32 configurations; nothing includes it,
# so it is excluded here.

set(BRIMSTONE_SOURCES
    Brimstone.cpp
    BrimstoneDoc.cpp
    BrimstoneView.cpp
    DvhViewHtml.cpp
    MainFrm.cpp
    OptGraphHtml.cpp
    OptThread.cpp
    PlanSetupDlg.cpp
    PlanarView.cpp
    PrescriptionToolbar.cpp
    SeriesDicomImporter.cpp
    WebView2Host.cpp
    StdAfx.cpp
)

set(BRIMSTONE_HEADERS
    Brimstone.h
    BrimstoneDoc.h
    BrimstoneView.h
    DvhViewHtml.h
    MainFrm.h
    OptGraphHtml.h
    OptThread.h
    PlanSetupDlg.h
    PlanarView.h
    PrescriptionToolbar.h
    Resource.h
    SeriesDicomImporter.h
    WebView2Host.h
    StdAfx.h
)

set(BRIMSTONE_RESOURCES
    Brimstone.rc
    res/Brimstone.rc2
)

# --- dependencies only Brimstone needs -----------------------------------------
find_package(DCMTK CONFIG REQUIRED)

# WebView2 (vcpkg port "webview2"): headers + WebView2Loader.dll.lib. The
# loader lib is also auto-linked by a #pragma in WebView2Host.cpp; finding it
# here puts its directory on the link path and lets vcpkg's applocal step
# deploy WebView2Loader.dll.
find_path(WEBVIEW2_INCLUDE_DIR WebView2.h)
find_library(WEBVIEW2_LOADER_LIBRARY NAMES WebView2Loader.dll.lib WebView2Loader.dll)
if(NOT WEBVIEW2_INCLUDE_DIR OR NOT WEBVIEW2_LOADER_LIBRARY)
    message(FATAL_ERROR "Brimstone: WebView2 SDK not found (vcpkg port 'webview2')")
endif()

# --- target --------------------------------------------------------------------
add_executable(Brimstone WIN32
    ${BRIMSTONE_SOURCES}
    ${BRIMSTONE_HEADERS}
    ${BRIMSTONE_RESOURCES}
)

target_include_directories(Brimstone PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/XMLLogging        # header-only use (XMLLogging.h)
    ${WEBVIEW2_INCLUDE_DIR}
)

target_link_libraries(Brimstone PRIVATE
    RtModel
    Graph
    DCMTK::dcmimgle
    DCMTK::dcmdata
    DCMTK::ofstd
    ${WEBVIEW2_LOADER_LIBRARY}
    ws2_32
    netapi32
)

target_compile_definitions(Brimstone PRIVATE
    _WINDOWS
    SBYTE_DEFINED
)

# Unicode MFC entry point (CMAKE_MFC_FLAG alone leaves the CRT looking for
# WinMain). WarpTps does the same.
set_target_properties(Brimstone PROPERTIES
    LINK_FLAGS "/ENTRY:wWinMainCRTStartup"
)

target_precompile_headers(Brimstone PRIVATE StdAfx.h)

# --- deploy next to the exe (vcxproj post-build events) --------------------------
# Kernel .dat files next to the exe.
file(GLOB DH_KERNEL_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*kernel.dat")
add_custom_command(TARGET Brimstone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DH_KERNEL_FILES} $<TARGET_FILE_DIR:Brimstone>
    COMMENT "Brimstone: deploying kernel .dat files"
    VERBATIM
)

# vtk.js planar-view assets, served through a virtual host mapped to
# <exe dir>\webassets\planar. The directory arrives with the WebView2 planar
# view (PR #53); until that merges it is absent on main, so copy only if it
# exists at configure time.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/webassets")
    add_custom_command(TARGET Brimstone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/webassets" "$<TARGET_FILE_DIR:Brimstone>/webassets"
        COMMENT "Brimstone: deploying webassets"
        VERBATIM
    )
endif()

# IPP runtime DLLs (ipps*, ippcore*) from the oneAPI bin dir, as the vcxproj
# post-build copies them. vcpkg's applocal deploy handles the vcpkg DLLs
# (ITK, DCMTK, WebView2Loader).
if(DH_IPP_FOUND AND IPP_BIN_DIR)
    file(GLOB DH_IPP_DLLS "${IPP_BIN_DIR}/ipps*.dll" "${IPP_BIN_DIR}/ippcore*.dll")
    if(DH_IPP_DLLS)
        add_custom_command(TARGET Brimstone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DH_IPP_DLLS} $<TARGET_FILE_DIR:Brimstone>
            COMMENT "Brimstone: deploying IPP runtime DLLs"
            VERBATIM
        )
    endif()
endif()
