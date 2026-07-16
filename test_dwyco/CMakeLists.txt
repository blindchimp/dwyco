cmake_minimum_required(VERSION 3.14)
project(dwytest LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DWYCO_CONFDIR test_dwyco_cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)
include(subdirs.cmake)

include_directories(
    ${VCCFG_COMP}
    ../bld/cdc32
    ../bld/vc
    ../bld/dwcls
    ../bld/kazlib
    ../bld/jenkins
    ../bld/zlib
    ../bld/crypto8
    ../bld/uv/include
    ../bld/toxd
)

set(BASELIBS
    cdc32
    vc
    crypto8
    dwcls
    gsm
    kazlib
    ppm
    pgm
    theora.1.2.x
    speex
    libvorbis-1.3.7
    ogg
    jenkins
    uv
    jhead
    miniupnpc
    toxd
)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    list(APPEND BASELIBS -ldl)
endif()
if(APPLE)
    list(APPEND BASELIBS -ldl)
endif()

add_executable(dwytest_local test_main.cpp )
target_link_libraries(dwytest_local PRIVATE ${BASELIBS})

add_executable(dwytest_peer test_peer.cpp )
target_link_libraries(dwytest_peer PRIVATE ${BASELIBS})
