cmake_minimum_required(VERSION 3.5)

project(xferbld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/crypto5 ${CMAKE_CURRENT_BINARY_DIR}/bld/crypto5)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/kazlib ${CMAKE_CURRENT_BINARY_DIR}/bld/kazlib)

add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/jenkins ${CMAKE_CURRENT_BINARY_DIR}/bld/jenkins)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/dwcls ${CMAKE_CURRENT_BINARY_DIR}/bld/dwcls)
add_subdirectory(${CMAKE_SOURCE_DIR}/../bld/vc ${CMAKE_CURRENT_BINARY_DIR}/bld/vc)
add_subdirectory(${CMAKE_SOURCE_DIR}/../xfer ${CMAKE_CURRENT_BINARY_DIR}/xfer)


#install(TARGETS xferbld
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
