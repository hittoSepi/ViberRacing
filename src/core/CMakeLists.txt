add_library(vr_core STATIC
    window.cpp
    input.cpp
    config.cpp
    logger.cpp
)

target_include_directories(vr_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(vr_core PUBLIC
    glfw
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

target_compile_definitions(vr_core PUBLIC
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_ENABLE_EXPERIMENTAL
)

if(UNIX AND NOT APPLE)
    target_link_libraries(vr_core PUBLIC dl pthread)
endif()
