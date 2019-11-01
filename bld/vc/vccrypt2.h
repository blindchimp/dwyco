
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef NO_VCCRYPTO
#ifndef VCCRYPT2_H
#define VCCRYPT2_H
// $Header: g:/dwight/repo/vc/rcs/vccrypt.h 1.4 1998/12/26 07:43:27 dwight Exp $
#include "vc.h"
vc vclh_sha(vc);
vc vclh_sha256(vc);
vc vclh_sha3_256(vc s);
vc vclh_md5(vc);
vc vclh_base64_encode(vc s);
vc vclh_base64_decode(vc s);
vc vclh_dh_init(vc, vc = vcnil);
vc vclh_dh_init_std();
vc vclh_dh_save(vc);
vc vclh_dh_setup();
vc vclh_dh_agree(vc);
vc lh_bf_init(vc);
vc lh_bf_init_key_stream(vc ke, vc kd);
vc lh_bf_init_key(vc k); // ECB mode
vc lh_bf_enc(vc);
vc lh_bf_dec(vc);
vc lh_dh_pubkey(vc);
vc lh_dh_pubkey_enc(vc, vc);
vc lh_dh_pubkey_dec(vc, vc);
vc lh_dh_pubkey_setup(vc);
vc lh_bf_init_key_cbc(vc key, vc iv);
vc lh_bf_xfer_enc(vc v);
vc bf_xfer_dec(vc v, vc& out);
vc lh_bf_xfer_dec(vc v, vc out);
vc vclh_dsa_init(vc file);
vc vclh_dsa_save(vc priv_filename, vc pub_filename);
vc vclh_dsa_pub_init(vc pub_filename);
vc vclh_dsa_sign(vc m);
vc vclh_dsa_verify(vc m, vc sig);

vc vclh_bf_open();
vc vclh_bf_close(vc ctx);
vc vclh_bf_init_key_stream_ctx(vc ctx, vc key1, vc key2);
vc vclh_bf_enc_ctx(vc ctx, vc v);
vc vclh_bf_dec_ctx(vc ctx, vc v);
vc vclh_bf_xfer_dec_ctx(vc ctx, vc v, vc out);
vc vclh_bf_xfer_enc_ctx(vc ctx, vc v);
vc vclh_bf_init_key_cbc_ctx(vc ctx, vc key, vc iv);
vc bf_xfer_dec_ctx(vc ctx, vc v, vc& out);

vc vclh_encdec_open();
vc vclh_encdec_close(vc ctx);
vc vclh_encdec_init_key_ctx(vc ctx, vc key, vc iv);
vc vclh_encdec_enc_ctx(vc ctx, vc v);
vc vclh_encdec_dec_ctx(vc ctx, vc v);
vc vclh_encdec_xfer_dec_ctx(vc ctx, vc v, vc out);
vc vclh_encdec_xfer_enc_ctx(vc ctx, vc v);
vc encdec_xfer_dec_ctx(vc ctx, vc v, vc& out);


#endif
#endif
