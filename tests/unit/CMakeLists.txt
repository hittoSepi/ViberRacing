add_executable(vr_tests_unit
    test_math.cpp
    test_spline.cpp
    test_serialization.cpp
)

target_link_libraries(vr_tests_unit PRIVATE
    vr_utils
    Catch2::Catch2WithMain
)

catch_discover_tests(vr_tests_unit)
