
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/rawconv.cc 1.15 1999/01/10 16:09:44 dwight Checkpoint $
#include "memory.h"
#include "rawconv.h"

RawConverter::RawConverter(int decom)
    : AudioConverter(decom)
{
}

int
RawConverter::convert(DWBYTE *buf, int len, int , DWBYTE*& out_buf, int& out_len)
{
    out_buf = new DWBYTE[len];
    out_len = len;
    memmove(out_buf, buf, len);
    return 1;
}
