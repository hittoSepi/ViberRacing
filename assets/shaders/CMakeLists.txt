# Shader compilation for bgfx.
# Shader binaries must be produced by bgfx's official shaderc tool.

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SHADER_SRC_DIR ${SHADER_DIR}/src)
set(SHADER_OUT_DIR ${SHADER_DIR})
set(BGFX_DIR ${CMAKE_SOURCE_DIR}/third_party/bgfx)
set(BX_DIR ${CMAKE_SOURCE_DIR}/third_party/bx)

find_program(SHADERC_EXECUTABLE
    NAMES shaderc shadercRelease shadercDebug
    PATHS
        ${BGFX_DIR}/.build/win64_vs2022/bin
        ${BGFX_DIR}/tools/bin/windows
        /usr/bin
        /usr/local/bin
    DOC "bgfx shader compiler"
)

if(WIN32 AND NOT SHADERC_EXECUTABLE)
    set(BGFX_SHADERC_EXECUTABLE ${BGFX_DIR}/.build/win64_vs2022/bin/shadercRelease.exe)
    find_program(BGFX_GENIE_EXECUTABLE
        NAMES genie.exe
        PATHS ${BX_DIR}/tools/bin/windows
        NO_DEFAULT_PATH
    )
    find_program(MSBUILD_EXECUTABLE
        NAMES MSBuild.exe
        PATHS
            "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/amd64"
            "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin"
            "C:/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin/amd64"
            "C:/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin"
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin/amd64"
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin"
    )

    if(NOT BGFX_GENIE_EXECUTABLE)
        message(FATAL_ERROR "bgfx shader tool bootstrap requires genie.exe at ${BX_DIR}/tools/bin/windows")
    endif()

    if(NOT MSBUILD_EXECUTABLE)
        message(FATAL_ERROR "bgfx shader tool bootstrap requires MSBuild.exe")
    endif()

    add_custom_command(
        OUTPUT ${BGFX_SHADERC_EXECUTABLE}
        COMMAND ${BGFX_GENIE_EXECUTABLE} --with-tools vs2022
        COMMAND ${MSBUILD_EXECUTABLE} .build\\projects\\vs2022\\shaderc.vcxproj /p:Configuration=Release /p:Platform=x64 /m
        WORKING_DIRECTORY ${BGFX_DIR}
        COMMENT "Bootstrapping bgfx shaderc tool"
    )

    add_custom_target(bgfx_shaderc_tool DEPENDS ${BGFX_SHADERC_EXECUTABLE})
    set(SHADERC_EXECUTABLE ${BGFX_SHADERC_EXECUTABLE})
elseif(SHADERC_EXECUTABLE)
    add_custom_target(bgfx_shaderc_tool)
else()
    message(FATAL_ERROR "shaderc not found. Build bgfx shaderc or provide it in PATH.")
endif()

set(SHADERS
    vs_basic
    fs_basic
    vs_textured
    fs_textured
    vs_atmosphere
    fs_atmosphere
    vs_skybox
    fs_skybox
)

# Match the runtime renderer selection in src/renderer/renderer.cpp.
if(WIN32)
    set(SHADER_PLATFORM "linux")
    set(SHADER_PROFILE_VS "120")
    set(SHADER_PROFILE_FS "120")
elseif(APPLE)
    set(SHADER_PLATFORM "linux")
    set(SHADER_PROFILE_VS "120")
    set(SHADER_PROFILE_FS "120")
else()
    set(SHADER_PLATFORM "linux")
    set(SHADER_PROFILE_VS "spirv")
    set(SHADER_PROFILE_FS "spirv")
endif()

foreach(SHADER ${SHADERS})
    set(SHADER_SRC ${SHADER_SRC_DIR}/${SHADER}.sc)
    set(SHADER_OUT ${SHADER_OUT_DIR}/${SHADER}.bin)

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
        DEPENDS
            bgfx_shaderc_tool
            ${SHADER_SRC}
            ${SHADER_SRC_DIR}/varying.def.sc
        COMMENT "Compiling ${SHADER} (${SHADER_PLATFORM}/${SHADER_PROFILE})"
    )

    list(APPEND SHADER_BINARIES ${SHADER_OUT})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SHADER_BINARIES})
