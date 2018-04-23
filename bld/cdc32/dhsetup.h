
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DHSETUP_H
#define DHSETUP_H

void dh_init();
int dh_load_account(const char *acct);
vc dh_gen_combined_keys();
vc dh_my_combined_publics();
vc dh_static_material(vc combined_material);
vc dh_my_static();
extern vc Server_publics;
extern vc Current_session_key;
extern vc Current_authenticator;

#endif
