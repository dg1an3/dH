# MODEL_BASE library

add_library(MODEL_BASE STATIC
    ModelObject.cpp
    Observer.cpp
    StdAfx.cpp
)

target_include_directories(MODEL_BASE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Precompiled header support
# target_precompile_headers(MODEL_BASE PRIVATE StdAfx.h)
