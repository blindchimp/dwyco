cmake_minimum_required(VERSION 3.14)

project(vccmd LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DWYCO_CONFDIR vccmd)
include(${CMAKE_CURRENT_SOURCE_DIR}/conf.cmake)

# order matters: zlib must come before vc because zlib generates
# zconf.h at build time.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/zlib ${CMAKE_CURRENT_BINARY_DIR}/bld/zlib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/kazlib ${CMAKE_CURRENT_BINARY_DIR}/bld/kazlib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/jenkins ${CMAKE_CURRENT_BINARY_DIR}/bld/jenkins)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/dwcls ${CMAKE_CURRENT_BINARY_DIR}/bld/dwcls)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/crypto8 ${CMAKE_CURRENT_BINARY_DIR}/bld/crypto8)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/uv ${CMAKE_CURRENT_BINARY_DIR}/bld/uv)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc ${CMAKE_CURRENT_BINARY_DIR}/bld/vc)

add_dependencies(vc zlibstatic)

# bld/zlib/zconf.h in the source tree is a stale artifact whose
# Z_HAVE_UNISTD_H is left as an unexpanded "#cmakedefine". Because zlib.h
# includes it with #include "zconf.h", the stale header shadows the one
# CMake generates, so unistd.h is never pulled in and lseek() is
# undeclared on macOS. Force-include the generated zconf.h to bypass it.
if(APPLE)
    set(_zconf ${CMAKE_CURRENT_BINARY_DIR}/bld/zlib/zconf.h)
    if(EXISTS "${_zconf}")
        target_compile_options(zlibstatic PRIVATE -include "${_zconf}")
        target_compile_options(zlib PRIVATE -include "${_zconf}")
    endif()
    unset(_zconf)
endif()

if(UNIX AND NOT APPLE)
    set(SPREAD_PREFIX "$ENV{HOME}/git/build-lh" CACHE PATH "Spread install prefix (must contain lib/libspread.a and include/sp.h)")
    if(NOT EXISTS "${SPREAD_PREFIX}/lib/libspread.a")
        message(FATAL_ERROR "spread not found at ${SPREAD_PREFIX}/lib/libspread.a — build it with vccmd/build.sh or set -DSPREAD_PREFIX=...")
    endif()
    add_library(spread STATIC IMPORTED GLOBAL)
    set_target_properties(spread PROPERTIES IMPORTED_LOCATION "${SPREAD_PREFIX}/lib/libspread.a")
    target_include_directories(spread INTERFACE "${SPREAD_PREFIX}/include")
endif()

set(VCCMD_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc/vcrun.cpp
)

if(UNIX AND NOT APPLE)
    list(APPEND VCCMD_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc/hacked_sqlite3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc/hacked_spread.xml.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c
    )
endif()

if(APPLE)
    list(APPEND VCCMD_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc/hacked_sqlite3.cpp
    )
endif()

add_executable(vccmd ${VCCMD_SRCS})

target_include_directories(vccmd PRIVATE
    ${VCCFG_COMP}
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/dwcls
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc
    ${CMAKE_CURRENT_BINARY_DIR}/../bld/zlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/zlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/crypto8
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/kazlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/jenkins
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/uv/include
)

target_link_libraries(vccmd PRIVATE
    vc
    dwcls
    crypto8
    jenkins
    kazlib
    uv
    zlibstatic
)

if(UNIX AND NOT APPLE)
    target_link_libraries(vccmd PRIVATE spread pthread dl)
endif()

if(APPLE)
    target_link_libraries(vccmd PRIVATE "-framework Foundation" "-framework CoreServices" sqlite3 pthread dl)
endif()