add_executable(vr_tests_unit
    test_math.cpp
    test_spline.cpp
    test_serialization.cpp
)

target_include_directories(vr_tests_unit PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(vr_tests_unit PRIVATE
    vr_utils
    Catch2::Catch2WithMain
)

add_test(NAME vr_tests_unit COMMAND vr_tests_unit)
