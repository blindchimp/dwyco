
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PRFCACHE_H
#define PRFCACHE_H

void init_prf_cache();
void exit_prf_cache();
int load_profile(vc uid, vc& prf_out);
int save_profile(vc uid, vc prf);
void clean_profile_cache(int days_old, int max_left);
int prf_already_cached(vc uid);
void prf_force_check(vc uid);
void prf_invalidate(vc uid);
void prf_set_cached(vc uid);

#define PRF_PACK 0
#define PRF_MEDIA 1
#define PRF_CHKSUM 2
#define PRF_REVIEWED 3
#define PRF_REGULAR 4
#define PRF_IMAGE 5


#endif
