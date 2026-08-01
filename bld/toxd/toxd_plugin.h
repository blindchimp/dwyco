
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
#ifndef TOXD_PLUGIN_H
#define TOXD_PLUGIN_H

#include "vc.h"
#include <stdint.h>
#include <stddef.h>

struct ToxPlugin;

typedef void (*ToxpEventCB)(const char *type, const vc &args, void *userdata);

enum {
    TOXP_STATUS_OK = 0,
    TOXP_STATUS_NEEDS_PASSWORD = 1,
    TOXP_STATUS_FAILED = 2
};

ToxPlugin *toxp_init(const char *save_file, const uint8_t *password, int password_len,
                     ToxpEventCB cb, void *userdata, int *status_out);
void toxp_shutdown(ToxPlugin *p);
void toxp_save(ToxPlugin *p);
void toxp_iterate(ToxPlugin *p);

// import / password support
// read a tox save file, decrypt it if needed (src_pw), and verify it is a
// loadable tox save. on success *out_data (malloc'd) and *out_len are set,
// caller must free(*out_data).
int toxp_import_prepare(const char *src_path, const uint8_t *src_pw, int src_pw_len,
                        uint8_t **out_data, size_t *out_len,
                        char *err_buf, int err_buf_len);
// write validated save data to save_file, encrypting with dst_pw if non-empty.
int toxp_import_commit(const char *save_file, const uint8_t *data, size_t len,
                       const uint8_t *dst_pw, int dst_pw_len,
                       char *err_buf, int err_buf_len);
// set/clear the password used to encrypt this profile's save data
// (immediately re-encrypts the on-disk save). empty pw clears it.
int toxp_set_password(ToxPlugin *p, const uint8_t *pw, int pw_len);
int toxp_has_password(ToxPlugin *p);
// returns 1 if the file at path is a toxencryptsave-encrypted tox save
int toxp_file_is_encrypted(const char *path);

vc toxp_get_address(ToxPlugin *p);
vc toxp_get_self_pubkey(ToxPlugin *p);
vc toxp_get_name(ToxPlugin *p);
vc toxp_get_status_message(ToxPlugin *p);

int toxp_set_name(ToxPlugin *p, const char *name, int len);
int toxp_set_status_message(ToxPlugin *p, const char *msg, int len);
void toxp_set_user_status(ToxPlugin *p, const char *status);
vc toxp_get_user_status(ToxPlugin *p);

int toxp_friend_add(ToxPlugin *p, const vc &address, const vc &message, uint32_t *fn_out);
int toxp_friend_add_norequest(ToxPlugin *p, const vc &pubkey, uint32_t *fn_out);
int toxp_friend_delete(ToxPlugin *p, uint32_t fn);
vc toxp_friend_list(ToxPlugin *p);
vc toxp_friend_get_name(ToxPlugin *p, uint32_t fn);

int toxp_message_send(ToxPlugin *p, uint32_t fn, const vc &text, int is_action, uint32_t *mid_out, int *tox_err_out);
int toxp_typing_set(ToxPlugin *p, uint32_t fn, int typing);

int toxp_file_send(ToxPlugin *p, uint32_t fn, const vc &name, uint64_t size, uint32_t *fnum_out);
int toxp_file_send_data(ToxPlugin *p, uint32_t fn, uint32_t fnum, uint64_t pos, const vc &data);
int toxp_file_accept(ToxPlugin *p, uint32_t fn, uint32_t fnum, int *error_out);
int toxp_file_cancel(ToxPlugin *p, uint32_t fn, uint32_t fnum);

#endif
