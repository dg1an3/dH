# MODEL_BASE library

add_library(MODEL_BASE STATIC
    ModelObject.cpp
    Observer.cpp
    StdAfx.cpp
)

target_include_directories(MODEL_BASE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# MFC-specific definitions
target_compile_definitions(MODEL_BASE PUBLIC
    _AFXDLL
)

# Precompiled header support
# target_precompile_headers(MODEL_BASE PRIVATE StdAfx.h)
