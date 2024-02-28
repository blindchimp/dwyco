# raw groq generated
cmake_minimum_required(VERSION 3.27)
include(/Users/dwight/git/dwyco/phoobld/conf.cmake)

# Dump all variables
message("Dumping all variables:")
get_cmake_property(variableNames VARIABLES)
foreach(varName ${variableNames})
    message("${varName} = ${${varName}}")
endforeach()


project(dwcls VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dwcls STATIC
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

target_include_directories(dwcls PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../kazlib
)

target_compile_features(dwcls PRIVATE
  cxx_std_11
)

set_target_properties(dwcls PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS OFF
)

install(TARGETS dwcls DESTINATION lib)

