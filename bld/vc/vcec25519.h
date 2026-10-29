/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCEC25519_H
#define VCEC25519_H
#include "vc.h"
#ifndef NO_VCCRYPTO

// ed25519 signing: singleton/global-key API, mirrors the DSA-* functions
vc vclh_ed25519_init(vc file = vcnil);
vc vclh_ed25519_save(vc priv_filename, vc pub_filename);
vc vclh_ed25519_pub_init(vc pub_filename);
vc vclh_ed25519_sign(vc m);
vc vclh_ed25519_verify(vc m, vc sig);

// ed25519 signing: stateless, keys passed in/out as 32-byte BSTRINGs
vc vclh_ed25519_gen_key(vc entropy = vcnil);
vc vclh_ed25519_pub_from_priv(vc priv);
vc vclh_ed25519_sign_key(vc m, vc priv);
vc vclh_ed25519_verify_key(vc m, vc sig, vc pub);

// x25519 key agreement: singleton/global-key API, mirrors the DH-* functions
vc vclh_x25519_init(vc entropy = vcnil);
vc vclh_x25519_keygen();
vc vclh_x25519_agree(vc other_public);
vc vclh_x25519_save(vc filename);
vc vclh_x25519_load(vc filename);

// x25519 key agreement: stateless, keys passed in/out as 32-byte BSTRINGs
vc vclh_x25519_gen_key(vc entropy = vcnil);
vc vclh_x25519_pub_from_priv(vc priv);
vc vclh_x25519_agree_key(vc our_priv, vc other_public);

#endif
#endif