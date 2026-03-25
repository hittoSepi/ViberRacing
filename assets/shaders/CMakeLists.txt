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
    foreach(SHADER ${SHADERS})
        set(SHADER_SRC ${SHADER_SRC_DIR}/${SHADER}.sc)
        set(SHADER_OUT ${SHADER_OUT_DIR}/${SHADER}.bin)
        
        # Determine shader type from prefix
        string(SUBSTRING ${SHADER} 0 2 SHADER_PREFIX)
        if(SHADER_PREFIX STREQUAL "vs")
            set(SHADER_TYPE "vertex")
        else()
            set(SHADER_TYPE "fragment")
        endif()
        
        add_custom_command(
            OUTPUT ${SHADER_OUT}
            COMMAND ${SHADERC_EXECUTABLE}
                -f ${SHADER_SRC}
                -o ${SHADER_OUT}
                --type ${SHADER_TYPE}
                --platform linux
                -p spirv
                -i ${CMAKE_SOURCE_DIR}/third_party/bgfx/src
                --varyingdef ${SHADER_SRC_DIR}/varying.def.sc
            DEPENDS ${SHADER_SRC} ${SHADER_SRC_DIR}/varying.def.sc
            COMMENT "Compiling ${SHADER}"
        )
        
        list(APPEND SHADER_BINARIES ${SHADER_OUT})
    endforeach()
    
    add_custom_target(shaders ALL DEPENDS ${SHADER_BINARIES})
else()
    # Create placeholder target
    add_custom_target(shaders 
        COMMAND ${CMAKE_COMMAND} -E echo "WARNING: shaderc not found, skipping shader compilation"
    )
endif()
