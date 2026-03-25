cmake_minimum_required(VERSION 3.20)

project(ViberRacing
    VERSION 0.1.0
    DESCRIPTION "3D Racing Game with Collaborative Track Editor"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VIBER_BUILD_TESTS "Build test suite" ON)
option(VIBER_BUILD_TOOLS "Build shader compiler tools" ON)
option(VIBER_ENABLE_PROFILING "Enable profiling support" OFF)

include(GNUInstallDirs)
include(cmake/CompilerOptions.cmake)

set(VR_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

add_library(vr_third_party INTERFACE)
target_include_directories(vr_third_party INTERFACE
    ${VR_THIRD_PARTY_DIR}/glm
    ${VR_THIRD_PARTY_DIR}/stb
    ${VR_THIRD_PARTY_DIR}
    ${VR_THIRD_PARTY_DIR}/json/single_include
)

add_subdirectory(${VR_THIRD_PARTY_DIR}/fmt fmt)
add_subdirectory(${VR_THIRD_PARTY_DIR}/spdlog spdlog)
add_subdirectory(${VR_THIRD_PARTY_DIR}/json json)

add_subdirectory(${VR_THIRD_PARTY_DIR}/glfw glfw)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(${VR_THIRD_PARTY_DIR}/assimp assimp)

set(BULLET_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BULLET_BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
set(BULLET_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
add_subdirectory(${VR_THIRD_PARTY_DIR}/bullet3 bullet3)

add_subdirectory(${VR_THIRD_PARTY_DIR}/enet enet)

set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(BX_DIR ${VR_THIRD_PARTY_DIR}/bx)
set(BIMG_DIR ${VR_THIRD_PARTY_DIR}/bimg)
# Strip /EHsc from default CXX flags for bx (uses SEH in thread.cpp)
set(_saved_cxx_flags "${CMAKE_CXX_FLAGS}")
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_subdirectory(${VR_THIRD_PARTY_DIR}/bx bx)
add_subdirectory(${VR_THIRD_PARTY_DIR}/bimg bimg)
add_subdirectory(${VR_THIRD_PARTY_DIR}/bgfx bgfx)
set(CMAKE_CXX_FLAGS "${_saved_cxx_flags}")

set(IMGUI_DIR ${VR_THIRD_PARTY_DIR}/imgui)
add_subdirectory(${VR_THIRD_PARTY_DIR}/imgui imgui)

set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
add_subdirectory(${VR_THIRD_PARTY_DIR}/freetype freetype)
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

set(RMLUI_TESTS OFF CACHE BOOL "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${VR_THIRD_PARTY_DIR}/RmlUi RmlUi)

add_subdirectory(assets/shaders)
add_subdirectory(src)

if(VIBER_BUILD_TOOLS)
    add_subdirectory(tools/car_editor)
endif()

if(VIBER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(${VR_THIRD_PARTY_DIR}/Catch2 Catch2)
    include(Catch)
    add_subdirectory(tests)
endif()

message(STATUS "")
message(STATUS "ViberRacing ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CXX compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build tests: ${VIBER_BUILD_TESTS}")
message(STATUS "")
