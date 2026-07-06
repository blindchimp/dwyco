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
    if(UNIX AND NOT APPLE)
	include_directories(/home/dwight/git/c-toxcore/toxcore)
        add_compile_definitions(LOCAL_TOXCORE)
    endif()
    if(APPLE)
        include_directories(/opt/homebrew/include)
    endif()
endif()

add_library(toxd OBJECT toxd.cpp)
