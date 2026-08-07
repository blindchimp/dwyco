
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

// encrypted-save support
// bridge remembers the last profile password in memory (never persisted).
int tox_bridge_needs_password();
int tox_bridge_unlock(const uint8_t *pw, int pw_len);
int tox_bridge_set_password(const uint8_t *pw, int pw_len);
int tox_bridge_has_password();
// verify that pw matches the on-disk profile password (1 if ok, or no
// password set). does not alter the running instance.
int tox_bridge_check_password(const uint8_t *pw, int pw_len);

// import a qTox-style .tox file as the active profile. the existing profile
// is validated/decrypted (src_pw), backed up to replaced_tox_save[.N].tox
// unless make_backup is 0, the live instance is replaced while running, and
// the imported save keeps its own password (empty src_pw leaves it
// unencrypted).
int tox_bridge_import_profile(const char *src_path, const uint8_t *src_pw, int src_pw_len,
                              int make_backup, char *err_buf, int err_buf_len);
// probe a tox save file: returns 1 if it is toxencryptsave-encrypted, 0 otherwise
int tox_bridge_file_is_encrypted(const char *path);

// export the live profile's save data to dst_path, preserving the current
// encryption state. on failure err_buf is filled with a message.
int tox_bridge_export_profile(const char *dst_path, char *err_buf, int err_buf_len);

// polling — call from dwyco timer loop
void tox_bridge_poll();

// identity
vc tox_bridge_get_address();
vc tox_bridge_get_pubkey();

// synced-profile management (group-shared tox saves via CRDT tags):
// publish the live profile's save bytes to the group; list available synced
// saves; activate one (replacing the local profile); resolve conflicts so only
// one device runs a given tox identity at a time.
int tox_bridge_publish_save();
vc tox_bridge_list_saves();
int tox_bridge_select_save(const vc &mid_hex, char *err_buf, int err_buf_len);
void tox_bridge_check_active_conflict();

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

// tox avatars: retrieves the cached avatar image data for a friend.
// returns 1 and fills avatar_data_out on success, 0 if none is available.
int tox_bridge_get_avatar(const vc &pseudo_uid, vc &avatar_data_out);

// tox avatars: set/clear our own avatar. set stores the image under both
// the app self uid and the tox self pseudo uid, then broadcasts it to all
// online friends (kind TOX_FILE_KIND_AVATAR, filename = tox_hash(image)).
// clear deletes the stored copies and signals removal with a size-0 avatar
// transfer.
int tox_bridge_set_avatar(const vc &avatar_data);
void tox_bridge_clear_avatar();

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
