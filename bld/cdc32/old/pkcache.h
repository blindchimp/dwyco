
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PKCACHE_H
#define PKCACHE_H

#include "vc.h"
void init_pk_cache();
void exit_pk_cache();
int get_pk(vc uid, vc& sfpk_out);
int get_pk2(vc uid, vc& sfpk_out, vc& alt_sfpk_out, vc &alt_name);
int put_pk(vc uid, vc prf, vc sig);
int put_pk2(vc uid, vc sfpk, vc sig, vc alt_pk, vc server_sig, vc gname);
void clean_pk_cache(int days_old, int max_left);

void pk_force_check(vc uid);
void pk_invalidate(vc uid);
void pk_set_session_cache(vc uid);
int pk_session_cached(vc uid);

#define PKC_STATIC_PUBLIC 0
#define PKC_DWYCO_SIGNATURE 1
#define PKC_ALT_STATIC_PUBLIC 2
#define PKC_ALT_SERVER_SIG 3
#define PKC_ALT_GNAME 4

#endif
