# XMLLogging library

add_library(XMLLogging STATIC
    XMLElement.cpp
    XMLLogFile.cpp
    XMLLoggableObject.cpp
    StdAfx.cpp
)

target_include_directories(XMLLogging PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(XMLLogging
    PUBLIC MTL
)

# Precompiled header support (optional, can be enabled if needed)
# target_precompile_headers(XMLLogging PRIVATE StdAfx.h)

# Add compile definitions
target_compile_definitions(XMLLogging PRIVATE
    USE_XMLLOGGING
)
