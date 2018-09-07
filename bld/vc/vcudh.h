
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCUDH_H
#define VCUDH_H
#include "vc.h"
#ifndef NO_VCCRYPTO

vc udh_init(vc entropy);
vc udh_gen_keys(vc udh_statics, vc entropy);
vc udh_agree_auth(vc our_material, vc other_publics);
vc udh_just_publics(vc);
vc udh_new_static(vc entropy);
vc dh_store_and_forward_material(vc other_pub, vc& session_key_out);
vc dh_store_and_forward_material2(vc other_pub_vec, vc& session_key_out, vc &key_validation);
vc dh_store_and_forward_get_key(vc sfpack, vc our_material);
vc vclh_sf_material(vc other_pub, vc key_out);

#define DH_STATIC_PUBLIC 0
#define DH_STATIC_PRIVATE 1

#endif
#endif
