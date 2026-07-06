cmake_minimum_required(VERSION 3.14)

project(dwycobg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DWYCO_CONFDIR dwycobg_bld)
set(DWYCOBG 1)

option(DWYCO_TOXCORE "Build with toxcore support" OFF)
if(DWYCO_TOXCORE)
    add_compile_definitions(DWYCO_TOXCORE)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)

if(NOT ANDROID)
add_subdirectory(../bld/crypto8 ${CMAKE_CURRENT_BINARY_DIR}/bld/crypto8)
add_subdirectory(../bld/kazlib ${CMAKE_CURRENT_BINARY_DIR}/bld/kazlib)
add_subdirectory(../bld/zlib ${CMAKE_CURRENT_BINARY_DIR}/bld/zlib)
add_subdirectory(../bld/jenkins ${CMAKE_CURRENT_BINARY_DIR}/bld/jenkins)
add_subdirectory(../bld/dwcls ${CMAKE_CURRENT_BINARY_DIR}/bld/dwcls)
add_subdirectory(../bld/pgm ${CMAKE_CURRENT_BINARY_DIR}/bld/pgm)
add_subdirectory(../bld/ppm ${CMAKE_CURRENT_BINARY_DIR}/bld/ppm)
add_subdirectory(../bld/toxd ${CMAKE_CURRENT_BINARY_DIR}/bld/toxd)
add_subdirectory(../bld/cdc32 ${CMAKE_CURRENT_BINARY_DIR}/bld/cdc32)
add_subdirectory(../bld/uv ${CMAKE_CURRENT_BINARY_DIR}/bld/uv)
add_subdirectory(../bld/vc ${CMAKE_CURRENT_BINARY_DIR}/bld/vc)
endif()

include_directories(
    ${VCCFG_COMP}
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/miscsrc
)

add_executable(dwycobg dwycobg.cpp)

set(BASELIBS cdc32 toxd vc crypto8 dwcls kazlib ppm pgm zlib jenkins uv)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(dwycobg PRIVATE ${BASELIBS} -lpthread -ldl)
endif()
if(APPLE)
    target_link_libraries(dwycobg PRIVATE ${BASELIBS} -lpthread -ldl)
endif()
if(WIN32)
    target_link_libraries(dwycobg PRIVATE ${BASELIBS} ws2_32.lib)
endif()

if(DWYCO_TOXCORE)
    if(UNIX AND NOT APPLE)
        target_link_libraries(dwycobg PRIVATE
            /home/dwight/git/c-toxcore/build/libtoxcore.a -lsodium)
    endif()
    if(APPLE)
        target_link_libraries(dwycobg PRIVATE
            /opt/homebrew/lib/libtoxcore.a /opt/homebrew/lib/libsodium.a)
    endif()
    if(WIN32)
        target_link_libraries(dwycobg PRIVATE toxcore.lib libsodium.lib ws2_32.lib)
    endif()
endif()
