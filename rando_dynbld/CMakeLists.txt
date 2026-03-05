cmake_minimum_required(VERSION 3.21)

project(dwyco_shared LANGUAGES CXX)
set(DWYCO_CONFDIR rando_dynbld)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(subdirs.cmake)

set(BASELIBS
    cdc32
    vc
    crypto8
    dwcls
    kazlib
    ppm
    pgm
    pbm
    jenkins
)

add_library(dwyco_jni SHARED dwyco.cpp)

target_link_options(dwyco_jni PRIVATE
    $<$<CONFIG:Release>:-s>
)

if(APPLE)

find_library(mumble CoreFoundation)
find_library(mumble2 ApplicationServices)

target_link_libraries(dwyco_jni PRIVATE
    ${BASELIBS}
    ${mumble}
    ${mumble2}
)
else()
    target_link_libraries(dwyco_jni PRIVATE
        ${BASELIBS}
    )
endif()
#target_link_libraries(dwyco PRIVATE ${mumble} ${mumble2})

#target_compile_definitions(dwyco_jni PRIVATE DWYCO_JNI_LIBRARY)
