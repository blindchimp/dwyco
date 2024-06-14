
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(v4lcap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(v4lcap PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../dwcls
  ${CMAKE_CURRENT_SOURCE_DIR}/../cdc32
)

add_library(v4lcap STATIC
v4l2cap.cpp
)


#install(TARGETS v4lcap DESTINATION lib)

