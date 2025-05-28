#ifdef DWYCO_DCT_CODER
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/packbits.cc 1.2 1997/11/25 20:41:03 dwight Stable095 $
 */

#ifdef RECORD
#include "dwlista.h"
DwListA<int> Symlist;
Packbits *CTracking;
#endif
Packbits::Packbits()
{
    bits_left = BITBUFSZ;
}
#endif
#include "packbits.h"

int
Unpackbits::getbit_raw(BITBUFT*& inbuf)
{
    int val;
    if(bits_left >= 1)
    {
        val = (cur >> (BITBUFSZ - 1)) & 1;
        --bits_left;
        cur <<= 1;
    }
    else
    {
        cur = *inbuf++;
        cur = int_to_le(cur);
#ifdef SHOW
        printf("%x load %x\n", this, cur);
#endif
        bits_left = BITBUFSZ - 1;
        val = (cur >> (BITBUFSZ - 1)) & 1;
        cur <<= 1;
    }
#ifdef SHOW
    printf("gotbit %x, %x, %d\n", this, inbuf, val);
#endif
    return val;
}


ELTYPE
Unpackbits::getbits_raw(int bits, BITBUFT*& inbuf)
{
    ELTYPE val;
    if(bits >= bits_left)
    {
        val = cur >> (BITBUFSZ - bits_left);
        val <<= (bits - bits_left);
        int morebits = bits - bits_left;
        cur = *inbuf++;
        cur = int_to_le(cur);
#ifdef SHOW
        printf("%x load %x\n", this, cur);
#endif
        val |= (cur >> (BITBUFSZ - (morebits))) & ((1 << (morebits)) - 1);
        bits_left = BITBUFSZ - (morebits);
        cur <<= morebits;
#ifdef SHOW
        printf("gotraw %d\n", val);
#endif
        return val;
    }
    val = cur >> (BITBUFSZ - bits);
    bits_left -= bits;
    cur <<= bits;
#ifdef SHOW
    printf("gotraw %d\n", val);
#endif
    return val;
}



void
Unpackbits::init(BITBUFT*& inbuf)
{
    cur = *inbuf++;
    cur = int_to_le(cur);
#ifdef SHOW
    printf("%x load %x\n", this, cur);
#endif
    bits_left = BITBUFSZ;
}

