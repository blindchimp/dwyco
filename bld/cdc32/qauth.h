
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/qauth.h 1.9 1999/01/10 16:11:01 dwight Checkpoint $
#ifndef QAUTH_H
#define QAUTH_H

#include "vc.h"
#include "dwstr.h"
int qauth_check_account_exists();
void init_qauth();
vc to_hex(vc s);
vc from_hex(vc);
vc gen_pass(vc phrase, vc& salt);
vc gen_id();
void qauth_get_password();
void save_auth_info(vc id, vc server_key, const char *filename);
int load_auth_info(vc& id, vc& server_key, const char *filename);
void add_entropy_timer(const char *astr, int alen);
void add_entropy(const char *astr, int alen);
vc get_entropy();
void init_entropy();
void save_entropy();
DwString gen_random_filename();
int gen_random_int();

extern vc My_UID;
extern vc My_MID;
extern vc My_server_key;
extern int Auth_remote;

#define ECHARGE 30

#endif
