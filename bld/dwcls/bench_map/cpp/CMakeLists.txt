cmake_minimum_required(VERSION 3.14)
set(DWYCO_CONFDIR phoo_cmake CACHE STRING "")
project(vcbench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(/home/dwight/git/dwyco/bld/kazlib ${CMAKE_BINARY_DIR}/kazlib_build)
add_subdirectory(/home/dwight/git/dwyco/bld/jenkins ${CMAKE_BINARY_DIR}/jenkins_build)
add_subdirectory(/home/dwight/git/dwyco/bld/crypto8 ${CMAKE_BINARY_DIR}/crypto8_build)
add_subdirectory(/home/dwight/git/dwyco/bld/uv ${CMAKE_BINARY_DIR}/uv_build)
add_subdirectory(/home/dwight/git/dwyco/bld/vc ${CMAKE_BINARY_DIR}/vc_build)
add_subdirectory(/home/dwight/git/dwyco/bld/dwcls ${CMAKE_BINARY_DIR}/dwcls_build)

add_executable(vc_bench cpp_bench_vc.cpp)
target_sources(vc_bench PRIVATE $<TARGET_OBJECTS:kazlib> $<TARGET_OBJECTS:jenkins> $<TARGET_OBJECTS:crypto8> $<TARGET_OBJECTS:uv> $<TARGET_OBJECTS:vc> $<TARGET_OBJECTS:dwcls>)
target_include_directories(vc_bench PRIVATE
    /home/dwight/git/dwyco/bld/vc
    /home/dwight/git/dwyco/bld/dwcls
    /home/dwight/git/dwyco/phoo_cmake
)
target_compile_definitions(vc_bench PRIVATE VC_INTERNAL NO_VCEVAL UNIX LINUX)
target_link_libraries(vc_bench PRIVATE pthread dl)
