add_library(vr_networking STATIC
    network.cpp
    p2p_host.cpp
    p2p_client.cpp
    sync.cpp
)

target_include_directories(vr_networking PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/third_party/enet/include
)

target_link_libraries(vr_networking PUBLIC
    vr_core
    vr_utils
    enet
)
