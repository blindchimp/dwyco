cmake_minimum_required(VERSION 3.14)

project(toxd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../vc
    ${CMAKE_CURRENT_SOURCE_DIR}/../dwcls
    ${CMAKE_CURRENT_SOURCE_DIR}/../kazlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../jenkins
    ${CMAKE_CURRENT_SOURCE_DIR}/../cdc32
)

if(DWYCO_TOXCORE)
    if(UNIX AND NOT APPLE AND NOT ANDROID)
	include_directories(/home/dwight/git/c-toxcore/toxcore)
        include_directories(/home/dwight/git/c-toxcore/toxencryptsave)
        add_compile_definitions(LOCAL_TOXCORE DWYCO_TOXCORE)
    endif()
    if(ANDROID)
        set(TOX_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../phoo_dynbld/tox-deps/${ANDROID_ABI})
        include_directories(${TOX_DEPS_DIR}/include)
        add_compile_definitions(DWYCO_TOXCORE SODIUM_STATIC)
    endif()
    if(APPLE)
        include_directories(/opt/homebrew/include)
    endif()
    if(WIN32)
        include_directories(C:/Users/dwight/c-toxcore/toxcore)
        include_directories(C:/Users/dwight/c-toxcore/toxencryptsave)
        include_directories(C:/Users/dwight/libsodium/src/libsodium/include)
        add_compile_definitions(LOCAL_TOXCORE DWYCO_TOXCORE SODIUM_STATIC)
    endif()
endif()

add_library(toxd OBJECT toxd.cpp)
