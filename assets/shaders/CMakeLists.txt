# Shader compilation for bgfx
# Requires shaderc to be built or available in PATH

find_program(SHADERC_EXECUTABLE 
    NAMES shaderc shadercRelease shadercDebug
    PATHS 
        ${CMAKE_SOURCE_DIR}/build/third_party/bgfx
        ${CMAKE_SOURCE_DIR}/third_party/bgfx/.build
        /usr/bin /usr/local/bin
    DOC "bgfx shader compiler"
)

if(NOT SHADERC_EXECUTABLE)
    message(WARNING "shaderc not found. Shaders need to be compiled manually.")
    message(STATUS "Build bgfx tools or install shaderc to compile shaders.")
endif()

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SHADER_SRC_DIR ${SHADER_DIR}/src)
set(SHADER_OUT_DIR ${SHADER_DIR})

set(SHADERS
    vs_basic
    fs_basic
    vs_textured
    fs_textured
)

set(SHADER_TYPE_map
    vs_basic:vertex
    fs_basic:fragment
    vs_textured:vertex
    fs_textured:fragment
)

if(SHADERC_EXECUTABLE)
    # Select platform and profile based on target OS and renderer
    if(WIN32)
        set(SHADER_PLATFORM "windows")
        set(SHADER_PROFILE_VS "vs_4_0")
        set(SHADER_PROFILE_FS "ps_4_0")
    elseif(APPLE)
        set(SHADER_PLATFORM "osx")
        set(SHADER_PROFILE_VS "metal")
        set(SHADER_PROFILE_FS "metal")
    else()
        set(SHADER_PLATFORM "linux")
        set(SHADER_PROFILE_VS "spirv")
        set(SHADER_PROFILE_FS "spirv")
    endif()

    foreach(SHADER ${SHADERS})
        set(SHADER_SRC ${SHADER_SRC_DIR}/${SHADER}.sc)
        set(SHADER_OUT ${SHADER_OUT_DIR}/${SHADER}.bin)

        # Determine shader type and profile from prefix
        string(SUBSTRING ${SHADER} 0 2 SHADER_PREFIX)
        if(SHADER_PREFIX STREQUAL "vs")
            set(SHADER_TYPE "vertex")
            set(SHADER_PROFILE ${SHADER_PROFILE_VS})
        else()
            set(SHADER_TYPE "fragment")
            set(SHADER_PROFILE ${SHADER_PROFILE_FS})
        endif()

        add_custom_command(
            OUTPUT ${SHADER_OUT}
            COMMAND ${SHADERC_EXECUTABLE}
                -f ${SHADER_SRC}
                -o ${SHADER_OUT}
                --type ${SHADER_TYPE}
                --platform ${SHADER_PLATFORM}
                -p ${SHADER_PROFILE}
                -i ${CMAKE_SOURCE_DIR}/third_party/bgfx/src
                --varyingdef ${SHADER_SRC_DIR}/varying.def.sc
            DEPENDS ${SHADER_SRC} ${SHADER_SRC_DIR}/varying.def.sc
            COMMENT "Compiling ${SHADER} (${SHADER_PLATFORM}/${SHADER_PROFILE})"
        )

        list(APPEND SHADER_BINARIES ${SHADER_OUT})
    endforeach()

    add_custom_target(shaders ALL DEPENDS ${SHADER_BINARIES})
else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_target(shaders ALL
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/pack_glsl_shaders.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Packing GLSL shaders (no shaderc found)"
    )
endif()
