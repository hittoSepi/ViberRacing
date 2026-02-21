add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(renderer)
add_subdirectory(assets)
add_subdirectory(physics)
add_subdirectory(networking)
add_subdirectory(game)

add_executable(ViberRacing main.cpp)

target_link_libraries(ViberRacing PRIVATE
    vr_core
    vr_renderer
    vr_physics
    vr_game
    vr_networking
    vr_assets
    vr_utils
)

set_target_properties(ViberRacing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(UNIX AND NOT APPLE)
    target_link_libraries(ViberRacing PRIVATE dl pthread)
endif()

if(APPLE)
    target_link_libraries(ViberRacing PRIVATE "-framework Cocoa" "-framework IOKit")
endif()
