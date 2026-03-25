add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(renderer)
add_subdirectory(assets)
add_subdirectory(physics)
add_subdirectory(networking)
add_subdirectory(game)

add_executable(ViberRacing main.cpp)

add_dependencies(ViberRacing shaders)

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

add_custom_command(TARGET ViberRacing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ViberRacing>/assets"
)

if(UNIX AND NOT APPLE)
    target_link_libraries(ViberRacing PRIVATE dl pthread)
endif()

if(APPLE)
    target_link_libraries(ViberRacing PRIVATE "-framework Cocoa" "-framework IOKit")
endif()
