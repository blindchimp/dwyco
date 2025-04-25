
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(pgm LANGUAGES C)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../pbm)
add_library(pgm OBJECT
	libpgm1.c
)


#install(TARGETS pgm DESTINATION lib)

