add_executable(vr_tests_integration
    test_physics_world.cpp
)

target_link_libraries(vr_tests_integration PRIVATE
    vr_core
    vr_physics
    vr_utils
    Catch2::Catch2WithMain
)

catch_discover_tests(vr_tests_integration)
