set(DWYCO_APP "vccmd")
set(LH_INTERPRETER 1)

set(VCCFG_COMP ${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR})

add_compile_definitions($<$<CONFIG:Debug>:DWYCO_DEBUG>)

if(NOT WIN32)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    add_compile_definitions(CRYPTOPP_DISABLE_ASM)
    add_compile_options(-Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-reorder)
endif()

if(UNIX AND NOT APPLE)
    add_compile_definitions(LINUX UNIX LH_WRAP_SPREAD LH_WRAP_SQLITE3 DWYCO_USE_STATIC_SQLITE)
endif()

if(APPLE)
    add_compile_definitions(MACOSX LINUX UNIX LH_WRAP_SQLITE3)
endif()

if(WIN32)
    add_compile_definitions(_WIN32 __WIN32__ _Windows)
    add_compile_options(/wd4100 /wd4068 /wd4189 /wd4291)
endif()