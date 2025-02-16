add_compile_definitions(-DVCCFG_FILE)
add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DEBUG>)

set(VCCFG_COMP ${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR})
# Determine the host platform
if(UNIX)
	add_definitions(-DLINUX)
	add_compile_options(-Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable -Wno-unused-function)
endif()

if(WIN32)
add_definitions(-D_WIN32 -D__WIN32__ -D_Windows)
add_compile_options(/wd4100 /wd4068 /wd4189 /wd4291)
endif()

if(APPLE)
message("FUCK")
    # Set compiler definitions for macOS
    add_definitions(-DMACOSX)
endif()


if(ANDROID)
    add_definitions(-DANDROID)
    add_compile_options( -frtti -fexceptions)
endif()

# Handle Emscripten-Specific Settings
if(EMSCRIPTEN)
    add_definitions(-DEMSCRIPTEN)
endif()

