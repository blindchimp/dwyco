
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(kazlib LANGUAGES C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DNDEBUG)

add_library(kazlib OBJECT
dict.c
)
