cmake_minimum_required(VERSION 3.14)
project(dwytest LANGUAGES CXX)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(DWYCO_TEST_APP_DIR="${CMAKE_CURRENT_SOURCE_DIR}/app")

set(DWYCO_CONFDIR test_dwyco_cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../${DWYCO_CONFDIR}/conf.cmake)
include(subdirs.cmake)

include_directories(
    ${VCCFG_COMP}
    ../bld/cdc32
    ../bld/vc
    ../bld/dwcls
    ../bld/kazlib
    ../bld/jenkins
    ../bld/zlib
    ../bld/crypto8
    ../bld/uv/include
    ../bld/toxd
)

set(BASELIBS
    cdc32
    vc
    crypto8
    dwcls
    gsm
    kazlib
    ppm
    pgm
    theora.1.2.x
    speex
    libvorbis-1.3.7
    ogg
    jenkins
    uv
    jhead
    miniupnpc
    toxd
)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    list(APPEND BASELIBS -ldl)
endif()
if(APPLE)
    list(APPEND BASELIBS -ldl)
endif()

add_executable(dwytest_local test_main.cpp dwyco_new_msg.cpp )
target_link_libraries(dwytest_local PRIVATE ${BASELIBS})

add_executable(dwytest_peer test_peer.cpp dwyco_new_msg.cpp )
target_link_libraries(dwytest_peer PRIVATE ${BASELIBS})

add_executable(dump_uid dump_uid.cpp )
target_link_libraries(dump_uid PRIVATE ${BASELIBS})

add_executable(add_pal add_pal.cpp )
target_link_libraries(add_pal PRIVATE ${BASELIBS})

add_executable(create_account create_account.cpp )
target_link_libraries(create_account PRIVATE ${BASELIBS})

add_executable(dwycobg ${CMAKE_CURRENT_SOURCE_DIR}/../dwycobg/dwycobg.cpp)
target_link_libraries(dwycobg PRIVATE ${BASELIBS})

add_executable(join_group join_group.cpp)
target_link_libraries(join_group PRIVATE ${BASELIBS})

add_executable(list_test_suite_part1 list_test_suite_part1.cpp)
target_link_libraries(list_test_suite_part1 PRIVATE ${BASELIBS})

add_executable(test_append_operations test_append_operations.cpp)
target_link_libraries(test_append_operations PRIVATE ${BASELIBS})

add_executable(test_creation_destruction test_creation_destruction.cpp)
target_link_libraries(test_creation_destruction PRIVATE ${BASELIBS})

add_executable(test_lists_12 test_lists_12.cpp)
target_link_libraries(test_lists_12 PRIVATE ${BASELIBS})

add_executable(list_helpers_test list_helpers_test.cpp)
target_link_libraries(list_helpers_test PRIVATE ${BASELIBS})
