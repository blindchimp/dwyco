set(DWYCOBG 0)
set(DWYCO_APP "dynlib")
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
add_compile_definitions(VCCFG_FILE DWYCO_APP_NICENAME="dwyco-generic")
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DEBUG>)

set(VCCFG_COMP ${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR})
add_compile_definitions( CRYPTOPP_MANUALLY_INSTANTIATE_TEMPLATES)
# Determine the host platform
if(UNIX)
        add_compile_definitions(LINUX)
        add_compile_options(-fPIC -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()

if(WIN32)
add_compile_definitions(_WIN32 __WIN32__ _Windows)
add_compile_options(/wd4100 /wd4068 /wd4189 /wd4291)
endif()

if(APPLE)
    # Set compiler definitions for macOS
    add_compile_definitions(MACOSX)
endif()


if(ANDROID)
    add_compile_definitions(ANDROID)
    add_compile_options( -frtti -fexceptions)
endif()

# Handle Emscripten-Specific Settings
if(EMSCRIPTEN)
    add_compile_definitions(EMSCRIPTEN)
endif()

