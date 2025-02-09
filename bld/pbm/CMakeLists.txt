
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(pbm LANGUAGES C)

add_library(pbm OBJECT
	libpbm1.c 
	libpbm2.c 
    libpbm4.c
)


#install(TARGETS pbm DESTINATION lib)

