
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(ppm LANGUAGES CXX)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../pbm ${CMAKE_CURRENT_SOURCE_DIR}/../pgm)

add_library(ppm OBJECT
	ppm.cpp
)



#install(TARGETS ppm DESTINATION lib)

