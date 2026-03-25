add_library(vr_renderer STATIC
    renderer.cpp
    camera.cpp
    mesh.cpp
    texture.cpp
    shader.cpp
    debug_draw.cpp
    ui/imgui_renderer.cpp
    shaders/embedded_shaders.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
)

target_include_directories(vr_renderer PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
)

target_link_libraries(vr_renderer PUBLIC
    vr_core
    vr_utils
    bgfx
    bimg
    bx
    imgui
    RmlUi::RmlUi
)
