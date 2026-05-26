
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
vc vclh_sha(VCArglist *a);
vc vclh_sha256(VCArglist *a);
vc vclh_sha3_256_std(VCArglist *a);
vc vclh_sha3_256_keccak(VCArglist *a);
vc vclh_md5(VCArglist *a);
vc vclh_base64_encode(const vc& s);
vc vclh_base64_decode(const vc& s);
vc vclh_dh_init(VCArglist *a);
vc vclh_dh_init_std();
vc vclh_dh_save(const vc&);
vc vclh_dh_setup();
vc vclh_dh_agree(const vc&);
vc lh_bf_init(const vc&);
vc lh_bf_init_key_stream(const vc& ke, const vc& kd);
vc lh_bf_init_key(const vc& k); // ECB mode
vc lh_bf_enc(const vc&);
vc lh_bf_dec(const vc&);
vc lh_dh_pubkey(const vc&);
vc lh_dh_pubkey_enc(const vc&, const vc&);
vc lh_dh_pubkey_dec(const vc&, const vc&);
vc lh_dh_pubkey_setup(const vc&);
vc lh_bf_init_key_cbc(const vc& key, const vc& iv);
vc lh_bf_xfer_enc(VCArglist *a);
vc bf_xfer_dec(const vc& v, vc& out);
vc lh_bf_xfer_dec(const vc& v, const vc& out);
vc vclh_dsa_init(const vc& file);
vc vclh_dsa_save(const vc& priv_filename, const vc& pub_filename);
vc vclh_dsa_pub_init(const vc& pub_filename);
vc vclh_dsa_sign(const vc& m);
vc vclh_dsa_verify(const vc& m, const vc& sig);

vc vclh_bf_open();
vc vclh_bf_close(VCArglist *a);
vc vclh_bf_init_key_stream_ctx(const vc& ctx, const vc& key1, const vc& key2);
vc vclh_bf_enc_ctx(const vc& ctx, const vc& v);
vc vclh_bf_dec_ctx(const vc& ctx, const vc& v);
vc vclh_bf_xfer_dec_ctx(const vc& ctx, const vc& v, const vc& out);
vc vclh_bf_xfer_enc_ctx(VCArglist *a);
vc vclh_bf_init_key_cbc_ctx(const vc& ctx, const vc& key, const vc& iv);
vc bf_xfer_dec_ctx(const vc& ctx, const vc& v, vc& out);

vc vclh_encdec_open();
vc vclh_encdec_close(VCArglist *a);
vc vclh_encdec_init_key_ctx(const vc& ctx, const vc& key, const vc& iv);
vc vclh_encdec_enc_ctx(const vc& ctx, const vc& v);
vc vclh_encdec_dec_ctx(const vc& ctx, const vc& v);
vc vclh_encdec_xfer_dec_ctx(const vc& ctx, const vc& v, const vc& out);
vc vclh_encdec_xfer_enc_ctx(VCArglist *a);
vc encdec_xfer_dec_ctx(const vc& ctx, const vc& v, vc& out, long mem_limit = -1);


#endif
#endif
