
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/rlc.h 1.2 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef RLC_H
#define RLC_H
#include "packbits.h"

#ifdef DWYCO_DCT_CODER
class BinaryRLC
{
public:
    BinaryRLC();

    void init();
    void addbit(int bit, BITBUFT*&);
    void flush(BITBUFT*&);

private:
    int cnt;
    int do_init;
    Packbits packer;
    int coding;

    void emit_run(int cnt, BITBUFT*&);
};
#endif

class BinaryRLD
{
public:
    BinaryRLD();

    void init(BITBUFT*&);
    int getbit(BITBUFT*&);

private:
    int cnt;
    int do_init;
    int decoding;
    Unpackbits unpacker;

    void read_run(BITBUFT*&);


};
#endif
