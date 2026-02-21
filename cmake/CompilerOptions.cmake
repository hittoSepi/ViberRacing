if(MSVC)
    add_compile_options(
        /W4
        /WX-
        /MP
        /EHsc
        /utf-8
    )
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -fPIC
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-unused-private-field)
    endif()
endif()

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(VIBER_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(VIBER_RELEASE)
else()
    add_compile_definitions(VIBER_RELEASE VIBER_SHIPPING)
endif()

if(VIBER_ENABLE_PROFILING)
    add_compile_definitions(VIBER_PROFILING_ENABLED)
endif()
