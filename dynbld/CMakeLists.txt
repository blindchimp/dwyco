cmake_minimum_required(VERSION 3.14)

project(dwyco LANGUAGES CXX)
set(DWYCO_CONFDIR dynbld)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(subdirs.cmake)

set(BASELIBS
    cdc32
    vc
    crypto5
    dwcls
    gsm
    kazlib
    ppm
    pgm
    pbm
    theora.1.2.x
    speex
    vorbis112
    ogg
    jenkins
    uv
    miniupnpc
)

add_library(dwyco SHARED dwyco.cpp)
target_link_libraries(dwyco
    ${BASELIBS}
)
target_link_options(dwyco PRIVATE "-Wl,--whole-archive")

#target_compile_definitions(dwyco_jni PRIVATE DWYCO_JNI_LIBRARY)
