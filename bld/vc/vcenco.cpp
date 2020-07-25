
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcenco.cpp 1.47 1997/10/05 17:27:06 dwight Stable $";

void oopanic(const char *);
#include "vcenco.h"

// use explicit offsets and whatnot for encoding items
// instead of "isdigit" and co. this is to avoid situations
// where an application decides it wants to use a non-standard
// locale.
#define ZERO 0x30
#define NINE 0x39

#define lisdigit(x) ((x) >= ZERO && (x) <= NINE)

static void strreverse(char* begin, char* end)
{
    char aux;
    while (end > begin) {
        aux = *end; *end-- = *begin; *begin++ = aux;
    }
}

int
pltoa(long value, char* str, int len, int field_width)
{
    char* wstr=str;
    char *endstr = &str[len];
    /* Conversion. Number is reversed. */
    do {
        if(wstr == endstr)
            oopanic("bad pltoa");
        *wstr++ = (char)(48 + (value % 10));
        value /= 10;
        --field_width;
    }
    while (value || field_width > 0);
    *wstr='\0';
    /* Reverse string */
    strreverse(str, wstr-1);
    return (wstr - str);
}

#if 0
// only handles positive input, used for serializing lengths that are
// always >=0
int
pltoa(long l, char *buf, int buf_len, int field_wid)
{
    char *tail = &buf[buf_len - 1];
    do
    {
        if(tail < buf)
            oopanic("bad pltoa");
        int rem = l % 10;
        *tail-- = rem + ZERO;
        l /= 10;
        --field_wid;
    }
    while(l != 0 || field_wid > 0);
    int len = &buf[buf_len - 1] - tail;
    memmove(buf, tail + 1, len);
    buf[len] = 0;
    return len;
}
#endif

// this handles all values of l, and produces
// output compatible with older versions (ie, the ones
// that use sprintf in the C locale).
int
compat_ltoa(long l, char *buf, int buf_len)
{
    int neg = (l < 0);

    char *tail = &buf[buf_len - 1];
    do
    {
        if(tail < buf)
            oopanic("bad compat_ltoa");
        int rem = l % 10;
        *tail-- = (neg ? -rem : rem ) + ZERO;
        l /= 10;
    }
    while(l != 0);
    if(neg)
    {
        if(tail < buf)
            oopanic("bad compat_ltoa");
        *tail-- = 0x2d;
    }

    int len = &buf[buf_len - 1] - tail;
    memmove(buf, tail + 1, len);
    buf[len] = 0;

    return len;
}

int
encode_long(char *buf, long l)
{
    if(l < 0)
        oopanic("bogus encode long");
    char buf3[40];
    char buf2[40];
    int bytes = pltoa(l, buf3, sizeof(buf3), -1);
    if(pltoa(bytes, buf2, sizeof(buf2), 2) != 2)
        oopanic("bad encode long");
    buf[0] = buf2[0];
    buf[1] = buf2[1];
    memcpy(buf + 2, buf3, bytes);
    return bytes + 2;
}

int
decode_len(char *buf)
{
    if(!lisdigit(buf[0]) || !lisdigit(buf[1]))
        return -1;
    return (buf[0] - ZERO) * 10 + (buf[1] - ZERO);
}

// decode a positive integer, return -1 on syntax error
// or overflow
long
decode_long(char *buf, int len)
{
    long l = 0;
    for(int i = 0; i < len; ++i)
    {
        if(!lisdigit(buf[i]))
            return -1;
        if(l > LONG_MAX / 10L)
            return -1;
        l *= 10L;
        int ndig = buf[i] - ZERO;
        if(l > LONG_MAX - ndig)
            return -1;
        l += ndig;
    }
    return l;
}

// decode a restricted format base-10 signed integer w/ overflow
// detection.
long
decode_long2(char *buf, int len, int& error)
{
    unsigned long l = 0;
    error = 0;
    int neg = 0;
    if(buf[0] == 0x2d)
    {
        neg = 1;
    }
    for(int i = (neg ? 1 : 0); i < len; ++i)
    {
        if(!lisdigit(buf[i]))
        {
            error = 1;
            return -1;
        }
        if(l > ULONG_MAX / 10L)
        {
            error = 1;
            return -1;
        }
        l *= 10UL;
        unsigned int ndig = buf[i] - ZERO;
        if(l > ULONG_MAX - ndig)
        {
            error = 1;
            return -1;
        }
        l += ndig;
    }
    if(neg)
    {
        if(l > ULONG_MAX / 2 + 1)
        {
            error = 1;
            return -1;
        }
        return -(long)l;
    }
    if(l > ULONG_MAX / 2)
    {
        error = 1;
        return -1;
    }
    return (long)l;
}

int
encode_type(char *buf, VCXFERTYPE t)
{
    if(t < 0 || t > 99)
        oopanic("bad type to encode");
    buf[0] = (t / 10) + ZERO;
    buf[1] = (t % 10) + ZERO;
    return 2;
}

VCXFERTYPE
decode_type(char *buf)
{
    if(!lisdigit(buf[0]) || !lisdigit(buf[1]))
        return -1;
    return (buf[0] - ZERO) * 10 + (buf[1] - ZERO);
}
