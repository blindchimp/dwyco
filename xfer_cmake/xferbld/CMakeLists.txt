cmake_minimum_required(VERSION 3.5)

project(xferbld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../xfer ${CMAKE_CURRENT_BINARY_DIR}/xfer)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../bld/vc ${CMAKE_CURRENT_BINARY_DIR}/vc)

#install(TARGETS xferbld
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
