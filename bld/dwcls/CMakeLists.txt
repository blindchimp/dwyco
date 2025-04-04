
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(dwcls LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(dwcls PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../kazlib
)



add_library(dwcls OBJECT
  dwhash.cpp
  useful.cpp
  dwstr.cpp
  dwtree2.cpp
  dwvp.cpp
  dwdate.cpp
  dwhist2.cc
  uricodec.cpp
  dwcls_timer.cpp
)

target_sources(dwcls INTERFACE dwvec.h dwvecp.h)
#install(TARGETS dwcls DESTINATION lib)

