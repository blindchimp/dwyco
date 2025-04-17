set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
set(DWYCO_APP "phoo")
set(DWYCOBG 0)
add_compile_definitions(DWYCO_APP_NICENAME="Phoo")

add_compile_definitions(VCCFG_FILE)
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DEBUG>)
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_APP_DEBUG>)
set(VCCFG_COMP ${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR})
add_compile_definitions(
	CDCCORE_STATIC
	DWYCO_FORCE_DESKTOP_VGQT
)
# Determine the host platform
if(UNIX AND NOT APPLE)
    add_compile_definitions(UNIX LINUX)
	add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()

if(WIN32)
add_compile_definitions(_WIN32 __WIN32__ _Windows)
add_compile_options(/wd4100 /wd4068 /wd4189 /wd4291)
endif()

if(APPLE)
    # Set compiler definitions for macOS
    add_compile_definitions(MACOSX LINUX UNIX)
	add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()
if(IOS)
    add_compile_definitions(DWYCO_IOS)
endif()


if(ANDROID)
    add_compile_definitions(ANDROID LINUX)
    add_compile_options( -frtti -fexceptions)
endif()

# Handle Emscripten-Specific Settings
if(EMSCRIPTEN)
    add_compile_definitions(EMSCRIPTEN)
endif()

