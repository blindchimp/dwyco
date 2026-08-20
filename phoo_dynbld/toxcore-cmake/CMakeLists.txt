# Minimal CMakeLists.txt for cross-compiling c-toxcore as a static library.
# Bypasses c-toxcore's own dependency resolution (which breaks under NDK
# cross-compilation) and compiles the source files directly.
#
# Required variables (set via -D):
#   TC_SRC   - path to c-toxcore source tree
#   SODIUM_INCLUDE_DIR - path to directory containing sodium.h

cmake_minimum_required(VERSION 3.16)
project(toxcore_cross C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Disable float packing in CMP (matches upstream)
add_definitions(-DCMP_NO_FLOAT=1)

set(TCX ${TC_SRC})

set(TOXCORE_SOURCES
    ${TCX}/third_party/cmp/cmp.c
    ${TCX}/toxcore/announce.c
    ${TCX}/toxcore/bin_pack.c
    ${TCX}/toxcore/bin_unpack.c
    ${TCX}/toxcore/ccompat.c
    ${TCX}/toxcore/crypto_core.c
    ${TCX}/toxcore/crypto_core_pack.c
    ${TCX}/toxcore/DHT.c
    ${TCX}/toxcore/ev.c
    ${TCX}/toxcore/events/conference_connected.c
    ${TCX}/toxcore/events/conference_invite.c
    ${TCX}/toxcore/events/conference_message.c
    ${TCX}/toxcore/events/conference_peer_list_changed.c
    ${TCX}/toxcore/events/conference_peer_name.c
    ${TCX}/toxcore/events/conference_title.c
    ${TCX}/toxcore/events/dht_nodes_response.c
    ${TCX}/toxcore/events/events_alloc.c
    ${TCX}/toxcore/events/file_chunk_request.c
    ${TCX}/toxcore/events/file_recv.c
    ${TCX}/toxcore/events/file_recv_chunk.c
    ${TCX}/toxcore/events/file_recv_control.c
    ${TCX}/toxcore/events/friend_connection_status.c
    ${TCX}/toxcore/events/friend_lossless_packet.c
    ${TCX}/toxcore/events/friend_lossy_packet.c
    ${TCX}/toxcore/events/friend_message.c
    ${TCX}/toxcore/events/friend_name.c
    ${TCX}/toxcore/events/friend_read_receipt.c
    ${TCX}/toxcore/events/friend_request.c
    ${TCX}/toxcore/events/friend_status.c
    ${TCX}/toxcore/events/friend_status_message.c
    ${TCX}/toxcore/events/friend_typing.c
    ${TCX}/toxcore/events/self_connection_status.c
    ${TCX}/toxcore/events/group_custom_packet.c
    ${TCX}/toxcore/events/group_custom_private_packet.c
    ${TCX}/toxcore/events/group_invite.c
    ${TCX}/toxcore/events/group_join_fail.c
    ${TCX}/toxcore/events/group_message.c
    ${TCX}/toxcore/events/group_moderation.c
    ${TCX}/toxcore/events/group_password.c
    ${TCX}/toxcore/events/group_peer_exit.c
    ${TCX}/toxcore/events/group_peer_join.c
    ${TCX}/toxcore/events/group_peer_limit.c
    ${TCX}/toxcore/events/group_peer_name.c
    ${TCX}/toxcore/events/group_peer_status.c
    ${TCX}/toxcore/events/group_privacy_state.c
    ${TCX}/toxcore/events/group_private_message.c
    ${TCX}/toxcore/events/group_self_join.c
    ${TCX}/toxcore/events/group_topic.c
    ${TCX}/toxcore/events/group_topic_lock.c
    ${TCX}/toxcore/events/group_voice_state.c
    ${TCX}/toxcore/forwarding.c
    ${TCX}/toxcore/friend_connection.c
    ${TCX}/toxcore/friend_requests.c
    ${TCX}/toxcore/group.c
    ${TCX}/toxcore/group_chats.c
    ${TCX}/toxcore/group_connection.c
    ${TCX}/toxcore/group_announce.c
    ${TCX}/toxcore/group_moderation.c
    ${TCX}/toxcore/group_onion_announce.c
    ${TCX}/toxcore/group_pack.c
    ${TCX}/toxcore/LAN_discovery.c
    ${TCX}/toxcore/list.c
    ${TCX}/toxcore/logger.c
    ${TCX}/toxcore/Messenger.c
    ${TCX}/toxcore/mem.c
    ${TCX}/toxcore/mono_time.c
    ${TCX}/toxcore/net.c
    ${TCX}/toxcore/net_crypto.c
    ${TCX}/toxcore/net_log.c
    ${TCX}/toxcore/net_profile.c
    ${TCX}/toxcore/network.c
    ${TCX}/toxcore/onion_announce.c
    ${TCX}/toxcore/onion.c
    ${TCX}/toxcore/onion_client.c
    ${TCX}/toxcore/os_event.c
    ${TCX}/toxcore/os_memory.c
    ${TCX}/toxcore/os_network.c
    ${TCX}/toxcore/os_random.c
    ${TCX}/toxcore/ping_array.c
    ${TCX}/toxcore/ping.c
    ${TCX}/toxcore/rng.c
    ${TCX}/toxcore/shared_key_cache.c
    ${TCX}/toxcore/sort.c
    ${TCX}/toxcore/state.c
    ${TCX}/toxcore/TCP_client.c
    ${TCX}/toxcore/TCP_common.c
    ${TCX}/toxcore/TCP_connection.c
    ${TCX}/toxcore/TCP_server.c
    ${TCX}/toxcore/timed_auth.c
    ${TCX}/toxcore/tox_api.c
    ${TCX}/toxcore/tox.c
    ${TCX}/toxcore/tox_dispatch.c
    ${TCX}/toxcore/tox_event.c
    ${TCX}/toxcore/tox_events.c
    ${TCX}/toxcore/tox_log_level.c
    ${TCX}/toxcore/tox_options.c
    ${TCX}/toxcore/tox_private.c
    ${TCX}/toxcore/tox_pack.c
    ${TCX}/toxcore/tox_unpack.c
    ${TCX}/toxcore/util.c
    ${TCX}/toxencryptsave/toxencryptsave.c
)

add_library(toxcore_static STATIC ${TOXCORE_SOURCES})
set_target_properties(toxcore_static PROPERTIES OUTPUT_NAME toxcore)

target_include_directories(toxcore_static
    PRIVATE
        ${TCX}
        ${TCX}/toxcore
        ${TCX}/toxencryptsave
        ${TCX}/third_party/cmp
        ${SODIUM_INCLUDE_DIR}
)

find_library(RT_LIBRARIES rt)
if(RT_LIBRARIES)
    target_link_libraries(toxcore_static PRIVATE ${RT_LIBRARIES})
endif()

find_package(Threads REQUIRED)
target_link_libraries(toxcore_static PRIVATE Threads::Threads)

# Install the static library and public headers
install(TARGETS toxcore_static ARCHIVE DESTINATION lib)
install(FILES ${TCX}/toxcore/tox.h         DESTINATION include/tox)
install(FILES ${TCX}/toxcore/tox_log_level.h DESTINATION include/tox)
install(FILES ${TCX}/toxcore/tox_options.h DESTINATION include/tox)
install(FILES ${TCX}/toxencryptsave/toxencryptsave.h DESTINATION include/tox)
