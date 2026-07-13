
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * WARNING: the bulk of this code is AI generated (opencode/big-pickle ca. 2026)
 */
#ifndef TOXBRIDGE_H
#define TOXBRIDGE_H

#include "vc.h"
#include "ser.h"
#include "dwstr.h"

namespace dwyco {

// NOTE: for this API, identifiers and addresses are all
// binary strings. there is no hex encoding.

// tox bridge lifecycle
int tox_bridge_init(const char *save_file);
void tox_bridge_shutdown();
int tox_bridge_is_active();
void tox_bridge_cleanup_incomplete();

// polling — call from dwyco timer loop
void tox_bridge_poll();

// identity
vc tox_bridge_get_address();
vc tox_bridge_get_pubkey();

// contact management
int tox_bridge_friend_add(const vc &address, const vc &message);
int tox_bridge_friend_add_norequest(const vc &pubkey);
int tox_bridge_friend_delete(uint32_t friend_number);
int tox_bridge_friend_delete_by_pubkey(const vc &pubkey);

// messaging
int tox_bridge_send_message(uint32_t friend_number, const vc &text, int is_action, uint32_t *mid_out = 0, int *tox_error_out = 0);
int tox_bridge_send_message_by_uid(const vc &pseudo_uid, const vc &text, int is_action);
int tox_bridge_send_file_message_by_uid(const vc &pseudo_uid, const vc &text,
                                        const vc &original_filename,
                                        const DwString &attachment_basename,
                                        const vc &filehash,
                                        uint64_t file_size,
                                        vc &local_mid_out);
void tox_bridge_send_queued();
int tox_bridge_kill_message(const vc &local_mid);

// file transfer
int tox_bridge_file_send(uint32_t friend_number, const vc &name, uint64_t size);
int tox_bridge_file_send_data(uint32_t friend_number, uint32_t file_number,
                              uint64_t pos, const vc &data);
int tox_bridge_file_accept(uint32_t friend_number, uint32_t file_number);

// pseudo-uid mapping
vc tox_pubkey_to_pseudo_uid(const vc &pubkey);
int tox_pseudo_uid_to_friend_number(const vc &pseudo_uid, uint32_t *fn_out);
int tox_friend_number_to_pseudo_uid(uint32_t fn, vc &pseudo_uid_out);

// friend number lookup (run on startup to rebuild cache)
void tox_bridge_rebuild_friend_cache();

// QD-compatible queue access for integrating tox messages into the
// dwyco_get_qd_messages / dwyco_qd_message_to_body APIs.
vc tox_queue_get_qd_msgs(const vc &pseudo_uid);
vc tox_queue_load_qd_body(const vc &local_mid);
int tox_queue_is_failed(const char *local_mid, int len);

// typing indicators
int tox_bridge_set_typing(uint32_t friend_number, int typing);
int tox_bridge_set_typing_by_uid(const vc &pseudo_uid, int typing);

// self profile
int tox_bridge_set_name(const char *name, int name_len);
int tox_bridge_set_status_message(const char *msg, int msg_len);
vc tox_bridge_get_name();
vc tox_bridge_get_status_message();

// user status (away/busy)
int tox_bridge_set_user_status(const char *status);
vc tox_bridge_get_user_status();

// convenience wrappers for dlli
int tox_bridge_is_tox_uid(const vc &uid);
vc make_tox_info_vec(const vc &pseudo, const vc &name);
// must call dwyco_free_array on returned pointer
int tox_bridge_get_self_public_key(char **out, int *len_out);
vc tox_bridge_get_friend_list_vc();

}

#endif
