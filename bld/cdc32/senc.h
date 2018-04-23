
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SENC_H
#define SENC_H
// $Header: g:/dwight/repo/cdc32/rcs/senc.h 1.2 1999/01/10 16:11:01 dwight Checkpoint $
#include "vc.h"

vc lh_bf_init(vc other);
vc lh_bf_enc(vc);
vc lh_bf_dec(vc);

void random_block(char *buf, int len);

#endif
