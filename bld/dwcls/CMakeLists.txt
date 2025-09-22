
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(dwcls LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(dwcls PRIVATE
  ${VCCFG_COMP}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../kazlib
  ${CMAKE_CURRENT_SOURCE_DIR}/../vc
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

target_sources(dwcls INTERFACE
    dwvec.h dwvecp.h dwsvec.h dwlista.h dwmapr.h dwqbm.h dwqbm2.h
    dwbag.h dwset.h dwycolist2.h dwassoc.h dwiter.h
)
#install(TARGETS dwcls DESTINATION lib)

