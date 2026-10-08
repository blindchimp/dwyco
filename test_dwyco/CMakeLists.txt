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

# Covers the list functions the other list tests never call:
# dwyco_list_print / _copy / _to_string / _from_string.
add_executable(dwytest_list dwytest_list.cpp)
target_link_libraries(dwytest_list PRIVATE ${BASELIBS})

# Covers the standalone crypto / string / password utilities. Needs no
# dwyco_init() and no server -- the entropy pool, the eze key and the file
# encryption context are all initialized lazily on first use.
add_executable(dwytest_utils dwytest_utils.cpp)
target_link_libraries(dwytest_utils PRIVATE ${BASELIBS})

# Settings / codec / runtime state / contacts. Needs dwyco_init() because the
# settings live in a set.sql that init creates, but does not need a login --
# registered under the "server" label anyway since it needs a client dir.
add_executable(dwytest_settings dwytest_settings.cpp)
target_link_libraries(dwytest_settings PRIVATE ${BASELIBS})

# Audio/mute/pause state plus the video-capture dispatch API. No peer, no
# call and no audio hardware needed, but dwyco_init() is required for the
# audio subsystem.
add_executable(dwytest_media_state dwytest_media_state.cpp)
target_link_libraries(dwytest_media_state PRIVATE ${BASELIBS})

# Video capture initialized from the raw (PPM) file input, so it needs no
# camera. This only links and runs when test_dwyco_cmake/conf.cmake sets
# DWYCO_TESTING=1; without it the raw-file path is compiled out and the test
# fails by design. Needs dwyco_init() for the settings map, but no peer.
add_executable(dwytest_vidcap_raw dwytest_vidcap_raw.cpp)
target_link_libraries(dwytest_vidcap_raw PRIVATE ${BASELIBS})

# User list, presence, uid resolution, trash, and the run_sql escape hatch.
add_executable(dwytest_users dwytest_users.cpp)
target_link_libraries(dwytest_users PRIVATE ${BASELIBS})

# Message tagging. Builds its own gi fixtures with dwyco_run_sql so the
# positive path does not need a second live process.
add_executable(dwytest_tags dwytest_tags.cpp)
target_link_libraries(dwytest_tags PRIVATE ${BASELIBS})

# Received-message API. Spawns itself as a second dwyco client so the tests
# have a real peer to receive from.
add_executable(dwytest_msg dwytest_msg.cpp dwyco_new_msg.cpp)
target_link_libraries(dwytest_msg PRIVATE ${BASELIBS})

# Attachment copy-out, views, previews, and forwarding. Spawns itself as a
# second dwyco client, as dwytest_msg does.
add_executable(dwytest_attach dwytest_attach.cpp dwyco_new_msg.cpp)
target_link_libraries(dwytest_attach PRIVATE ${BASELIBS})

# Profiles: one client sets a profile, the other fetches it. Spawns itself.
add_executable(dwytest_profile dwytest_profile.cpp dwyco_new_msg.cpp)
target_link_libraries(dwytest_profile PRIVATE ${BASELIBS})

# Call/channel state, pals-only filtering, selective chat, and callback
# registration. No live media session, so it needs no peer.
add_executable(dwytest_calls dwytest_calls.cpp)
target_link_libraries(dwytest_calls PRIVATE ${BASELIBS})

# Chat server, chat context, and the chat queue. Needs a directory server
# (which doubles as a chat server), but no peer.
add_executable(dwytest_chat dwytest_chat.cpp)
target_link_libraries(dwytest_chat PRIVATE ${BASELIBS})

# ---- ctest ----
#
# Previously every binary here was run by hand. What can be driven by ctest
# falls into two groups:
#
#  * The list data structure tests. No account, no network, no cwd state, so
#    they are registered unconditionally and are the ones to run first when
#    something breaks.
#
#  * The account tests. These need a live server plus a writable client dir,
#    and several need the uids of other accounts, which are not knowable at
#    configure time. dwytest_local falls back to the DWYTEST_DIR /
#    DWYTEST_PEER / DWYTEST_SEED environment variables for its arguments (see
#    arg_or_env() in test_common.h), so set those and ctest picks them up:
#
#      DWYTEST_DIR=/tmp/dwyctest \
#      DWYTEST_PEER=<20 hex chars> \
#      DWYTEST_SEED=<20 hex chars> \
#      ctest --output-on-failure
#
#    Values may come from the environment or from -D at configure time.
#    With no uids set, dwytest_local still runs; the peer-dependent tests
#    report SKIP rather than failing.

enable_testing()

foreach(v DIR PEER SEED)
    if(NOT DEFINED DWYTEST_${v})
        set(DWYTEST_${v} "$ENV{DWYTEST_${v}}")
    endif()
endforeach()

foreach(server_free_target
        list_test_suite_part1
        test_creation_destruction
        test_append_operations
        test_lists_12
        list_helpers_test
        dwytest_list
        dwytest_utils)
    add_test(NAME ${server_free_target} COMMAND ${server_free_target})
    set_tests_properties(${server_free_target} PROPERTIES LABELS "server-free")
endforeach()

foreach(account_target dwytest_local dwytest_settings dwytest_media_state dwytest_vidcap_raw dwytest_users dwytest_tags dwytest_msg dwytest_attach dwytest_profile dwytest_calls dwytest_chat)
    add_test(NAME ${account_target} COMMAND ${account_target})
    set_tests_properties(${account_target} PROPERTIES
        LABELS "server"
        TIMEOUT 300
        ENVIRONMENT
            "DWYTEST_DIR=${DWYTEST_DIR};DWYTEST_PEER=${DWYTEST_PEER};DWYTEST_SEED=${DWYTEST_SEED}")
endforeach()
