
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../${DWYCO_CONFDIR}/conf.cmake)

project(jhead LANGUAGES CXX)
include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/cjhead)
add_compile_options(-fpermissive)

add_library(jhead STATIC
exif.cpp 
gpsinfo.cpp 
iptc.cpp 
jhead.cpp 
jpgfile.cpp 
jpgqguess.cpp 
makernote.cpp 
paths.cpp 
)

#install(TARGETS jhead DESTINATION lib)

