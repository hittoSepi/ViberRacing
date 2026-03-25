add_library(vr_core STATIC
    window.cpp
    input.cpp
    config.cpp
)

target_include_directories(vr_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(vr_core PUBLIC
    glfw
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    vr_third_party
)

target_compile_definitions(vr_core PUBLIC
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_ENABLE_EXPERIMENTAL
)

if(UNIX AND NOT APPLE)
    target_link_libraries(vr_core PUBLIC dl pthread)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT wayland-client)
    if(WAYLAND_CLIENT_FOUND)
        target_link_libraries(vr_core PUBLIC ${WAYLAND_CLIENT_LIBRARIES})
        target_include_directories(vr_core PUBLIC ${WAYLAND_CLIENT_INCLUDE_DIRS})
    endif()
endif()
