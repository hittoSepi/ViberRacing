add_library(vr_utils STATIC
    spline.cpp
    profiler.cpp
)

target_include_directories(vr_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(vr_utils PUBLIC vr_core)
