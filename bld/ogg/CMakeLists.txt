
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(ogg LANGUAGES C)

include_directories(ogg PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(ogg STATIC
src/framing.c 
src/bitwise.c
)


#install(TARGETS ogg DESTINATION lib)

