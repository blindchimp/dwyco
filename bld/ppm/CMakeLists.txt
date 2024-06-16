
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(ppm LANGUAGES C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../pbm ${CMAKE_CURRENT_SOURCE_DIR}/../pgm)

add_library(ppm STATIC
	libppm1.c
    libppm2.c
)


#install(TARGETS ppm DESTINATION lib)

