add_executable(car_editor
    main.cpp
    editor_state.cpp
    editor_ui.cpp
)

add_dependencies(car_editor shaders)

target_link_libraries(car_editor PRIVATE
    vr_core
    vr_renderer
    vr_physics
    vr_assets
    vr_utils
    vr_game
)

target_include_directories(car_editor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(car_editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET car_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:car_editor>/assets"
)

if(UNIX AND NOT APPLE)
    target_link_libraries(car_editor PRIVATE dl pthread)
endif()

if(APPLE)
    target_link_libraries(car_editor PRIVATE "-framework Cocoa" "-framework IOKit")
endif()
