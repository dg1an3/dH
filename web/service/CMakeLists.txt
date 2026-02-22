cmake_minimum_required(VERSION 3.20)

set(SERVICE_SOURCES
    src/main.cpp
    src/BrimstoneServer.cpp
    src/Routes.cpp
    src/WebSocketHub.cpp
    src/adapter/PlanAdapter.cpp
    src/adapter/VolumeAdapter.cpp
    src/adapter/OptimizationJob.cpp
    src/adapter/ProgressCallback.cpp
)

set(SERVICE_HEADERS
    src/BrimstoneServer.h
    src/Routes.h
    src/WebSocketHub.h
    src/adapter/PlanAdapter.h
    src/adapter/VolumeAdapter.h
    src/adapter/OptimizationJob.h
    src/adapter/ProgressCallback.h
    src/util/JsonUtils.h
    src/util/BinaryWriter.h
)

add_executable(BrimstoneService ${SERVICE_SOURCES} ${SERVICE_HEADERS})

target_include_directories(BrimstoneService PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../RtModel/include
    ${CMAKE_SOURCE_DIR}/../Brimstone
)

# Link to RtModel library (built separately via Visual Studio)
target_link_directories(BrimstoneService PRIVATE
    ${CMAKE_SOURCE_DIR}/../x64/Release
    ${CMAKE_SOURCE_DIR}/../x64/Debug
)

target_link_libraries(BrimstoneService PRIVATE
    nlohmann_json::nlohmann_json
    ${ITK_LIBRARIES}
    optimized RtModel
    debug RtModeld
)

# Crow is header-only, just need the include path from vcpkg
find_path(CROW_INCLUDE_DIRS "crow.h")
target_include_directories(BrimstoneService PRIVATE ${CROW_INCLUDE_DIRS})

# Windows-specific settings
if(WIN32)
    target_compile_definitions(BrimstoneService PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0A00
    )
    target_link_libraries(BrimstoneService PRIVATE ws2_32 wsock32)
endif()

# Copy runtime dependencies
add_custom_command(TARGET BrimstoneService POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BrimstoneService>
        ${CMAKE_SOURCE_DIR}/../x64/$<CONFIG>/
)
