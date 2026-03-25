add_library(vr_game STATIC
    game.cpp
    states/null_state.cpp
    states/menu_state.cpp
    states/racing_state.cpp
    states/editor_state.cpp
    states/lobby_state.cpp
    entities/car.cpp
    entities/car_body.cpp
    entities/car_damage.cpp
    entities/atmosphere.cpp
    entities/ground_plane.cpp
    entities/track.cpp
    entities/checkpoint.cpp
    track_editor/editor.cpp
    track_editor/road_tool.cpp
    track_editor/terrain_tool.cpp
    track_editor/prop_tool.cpp
)

target_include_directories(vr_game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(vr_game PUBLIC
    vr_core
    vr_renderer
    vr_physics
    vr_networking
    vr_assets
    vr_utils
)
