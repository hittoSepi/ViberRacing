add_library(vr_physics STATIC
    world.cpp
    vehicle.cpp
    tire_model.cpp
)

target_include_directories(vr_physics PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/third_party/bullet3/src
)

target_link_libraries(vr_physics PUBLIC
    vr_core
    vr_utils
    BulletDynamics
    BulletCollision
    LinearMath
)
