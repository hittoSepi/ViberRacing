add_executable(vr_tests_integration
    test_physics_world.cpp
)

target_link_libraries(vr_tests_integration PRIVATE
    vr_core
    vr_physics
    vr_utils
    Catch2::Catch2WithMain
)

add_test(NAME vr_tests_integration COMMAND vr_tests_integration)
