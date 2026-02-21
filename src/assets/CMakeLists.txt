add_library(vr_assets STATIC
    manager.cpp
    mesh_loader.cpp
    texture_loader.cpp
)

target_include_directories(vr_assets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(vr_assets PUBLIC
    vr_core
    vr_renderer
    assimp::assimp
)
