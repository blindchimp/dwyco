cmake_minimum_required(VERSION 3.21)

project(ftpreview LANGUAGES CXX)
set(DWYCO_CONFDIR ftpreview)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)

include_directories(
    ${VCCFG_COMP}
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/dwcls
    ${CMAKE_CURRENT_SOURCE_DIR}/../bld/vc
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(subdirs.cmake)

set(BASELIBS
    cdc32
    vc
    crypto8
    dwcls
    gsm
    kazlib
    ppm
    pgm
    pbm
    theora.1.2.x
    speex
    libvorbis-1.3.7
    ogg
    jenkins
    uv
    miniupnpc
)

add_executable(ftpreview ftpreview.cpp)

target_link_options(ftpreview PRIVATE
    $<$<CONFIG:Release>:-s>
)

target_link_libraries(ftpreview PRIVATE
${BASELIBS}
)

