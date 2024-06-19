cmake_minimum_required(VERSION 3.14)

project(dwyco_jni LANGUAGES CXX)
set(DWYCO_CONFDIR phoo_ndk)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(subdirs.cmake)

add_library(dwyco_jni SHARED
""
)
target_link_libraries(dwyco_jni
    dwcls
)
target_compile_definitions(dwyco_jni PRIVATE DWYCO_JNI_LIBRARY)
