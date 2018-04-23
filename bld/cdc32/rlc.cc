
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/rlc.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include <stdlib.h>
#include "rlc.h"
void oopanic(const char *);

BinaryRLC::BinaryRLC()
{
    do_init = 1;
    cnt = 0;
    coding = 0;
    packer.init();
}

void
BinaryRLC::init()
{
    do_init = 1;
    cnt = 0;
    coding = 0;
    packer.init();
}

void
BinaryRLC::addbit(int bit, BITBUFT*& out)
{
    if(do_init)
    {
        packer.addbits_raw(bit, 1, out);
        do_init = 0;
        coding = bit;
        cnt = 0;
    }
    if(coding != bit)
    {
        emit_run(cnt, out);
        cnt = 1;
        coding = bit;
    }
    else
        ++cnt;
}

void
BinaryRLC::emit_run(int cnt, BITBUFT*& out)
{
    if(cnt == 0)
        oopanic("emit cnt 0");
    packer.addbits_raw((cnt < 4) ? cnt : 0, 2, out);
    if(cnt < 4)
        return;
    packer.addbits_raw((cnt < 16) ? cnt : 0, 4, out);
    if(cnt < 16)
        return;
    packer.addbits_raw((cnt < 64) ? cnt : 0, 6, out);
    if(cnt < 64)
        return;
    if(cnt < 8192)
        packer.addbits_raw(cnt, 13, out);
    else
        oopanic("run overflow");
}

void
BinaryRLC::flush(BITBUFT*& out)
{
    if(cnt != 0)
        emit_run(cnt, out);
    packer.flush(out);
}

BinaryRLD::BinaryRLD()
{
    do_init = 1;
    cnt = 0;
    decoding = 0;
}

void
BinaryRLD::init(BITBUFT*& in)
{
    do_init = 1;
    cnt = 0;
    decoding = 0;
    unpacker.init(in);
}

void
BinaryRLD::read_run(BITBUFT*& in)
{
    int t = unpacker.getbits_raw(2, in);
    if(t)
    {
        cnt = t;
        return;
    }
    t = unpacker.getbits_raw(4, in);
    if(t)
    {
        cnt = t;
        return;
    }
    t = unpacker.getbits_raw(6, in);
    if(t)
    {
        cnt = t;
        return;
    }
    t = unpacker.getbits_raw(13, in);
    if(t)
    {
        cnt = t;
        return;
    }
    oopanic("bad readrun");
}

int
BinaryRLD::getbit(BITBUFT*& in)
{
    if(do_init)
    {
        decoding = unpacker.getbits_raw(1, in);
        read_run(in);
        do_init = 0;
    }
    if(cnt == 0)
    {
        read_run(in);
        decoding = !decoding;
    }
    --cnt;
    return decoding;
}

#ifdef TESTRLC

BITBUFT s[100];
#define add(x, n) \
	for(i = 0; i < n; ++i) \
		a.addbit(x, ss);

#define check(x, n) \
	for(i = 0; i < n; ++i) \
		if(b.getbit(ss) != x) \
			::abort();
main()
{
    int i;
    BinaryRLC a;
    BinaryRLD b;
    BITBUFT * ss = s;

    a.init();
    add(0, 6);
    add(1, 500);
    add(0, 10);
    add(1, 1);
    add(1, 1);
    add(0, 1);
    add(1, 5);
    add(0, 2000);
    add(0, 255);
    add(1, 256);
    add(0, 3);
    add(1, 4);
    add(0, 15);
    add(1, 16);
    a.flush(ss);
    printf("%d bytes\n", (char *)ss - (char *)s);
    ss = s;
    b.init(ss);
    check(0, 6);
    check(1, 500);
    check(0, 10);
    check(1, 1);
    check(1, 1);
    check(0, 1);
    check(1, 5);
    check(0, 2000);
    check(0, 255);
    check(1, 256);
    check(0, 3);
    check(1, 4);
    check(0, 15);
    check(1, 16);
}
#endif
