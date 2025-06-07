
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PACKBITS_H
#define PACKBITS_H
// pack/unpack bits into a stream
// $Header: g:/dwight/repo/cdc32/rcs/packbits.h 1.8 1997/05/18 05:16:33 dwight Stable095 $
// note: this class is hokey, it has some quantization stuff
// in it along with the bit packing... but you can ignore it.
//
// NOTE: if you are not compiling this into the Dwyco Video
// Conferencing system, you'll have to #define PB_STANDALONE.
//
// These classes help you pack and unpack a stream of bits
// into a buffer. The buffer is controlled by the user of this
// class (ie. the class does *no* buffer management.)
// Before using these objects, you must have set up a memory
// buffer to recv the packed bit stream (in the case of Packbits).
// For Unpackbits, you must have a memory buffer filled
// with the stream of bits that was created by a Packbits object.
//
// To create a packed stream:
//
// Packbits pb;
// BITBUFT *buffer_ptr = your_buffer;
// pb.init();
// pb.addbits_raw(val, #bits, buffer_ptr)
// and so on.
// The bits to store away are assumed to be in the low
// order end of val.
// The Packbits object updates your pointer as it
// needs to, so you have to keep it around between
// calls to addbits. The Packbits object must also
// be kept in existence as long as you are creating
// a stream.
// After you have added the last bits, you must call
// pb.flush(buffer_ptr)
// To find out out much buffer space was finally used, you
// can subtract the resulting buffer_ptr and the pointer
// to the beginning of the buffer.
//
//
// To unpack the resulting stream:
//
// Unpackbits upb;
// BITBUFT *buffer_ptr = start_of_packed_buffer;
// upb.init(buffer_ptr);
// val = upb.getbits_raw(#bits, buffer_ptr);
// and so on.
//
// This class acts like the other: the object and
// the buffer pointer must be kept around as long
// as you are unpacking a stream.
//
//
//
//
#include <stdio.h>

#ifndef PB_STANDALONE
#include "matcom.h"
#else
typedef unsigned int BITBUFT;
typedef unsigned char DWBYTE;
#endif

#undef SHOW
#undef RECORD
#ifdef __ppc__
#define int_to_le(x) ((((x)&0xff) << 24)|(((x)&0xff00) << 8)|(((x)&0xff0000) >> 8)|(((x) >> 24)&0xff))
#else
#define int_to_le(x) (x)
#endif

#ifdef RECORD
class Packbits;
class JDUnpackbits;
extern Packbits *CTracking;
extern JDUnpackbits *DTracking;
#include "dwlista.h"
#include "vfwmgr.h"
extern DwListA<int> Symlist;
#define dorec(a) {if(this == CTracking) {Symlist.append(a);}}
#define shouldbe2(o, a) {\
	if((o) == DTracking) { \
		int k; \
		if(Symlist.eol()) { \
			printf("out of list elements\n"); \
			printf("decoding %d\n", (a)); \
			dwlista_foreach(k, Symlist) \
			{ \
				printf("%d\n", k); \
			} \
			delete TheVFWMgr; \
			fflush(stdout); \
			fclose(stdout); \
			::exit(1); \
		} \
		if(Symlist.get() != (a)) {\
			delete TheVFWMgr; \
			printf("decoding %d, list %d\n", (a), Symlist.get()); \
			dwlista_foreach(k, Symlist) \
			{ \
				printf("%d\n", k); \
			} \
			fflush(stdout); \
			fclose(stdout); \
			::exit(1); \
		} \
		else \
			Symlist.forward(); \
	} \
}

#define shouldbe(a) shouldbe2((this), (a))
#endif

#define BITBUFSZ (sizeof(cur) * 8)

#ifdef DWYCO_DCT_CODER
class Packbits
{
public:
    Packbits();

    inline void flush(BITBUFT*& outbuf);
    inline void flush(DWBYTE*& outbuf) {
        BITBUFT* t = (BITBUFT*)outbuf;
        flush(t);
        outbuf = (DWBYTE*) t;
    }
    inline void init();
    inline void addbits_raw(ELTYPE val, int bits, BITBUFT*& outbuf);
    inline void add0(BITBUFT*& outbuf);
    inline void add1(BITBUFT*& outbuf);
    inline void addbits_raw(ELTYPE val, int bits, DWBYTE*& outbuf) {
        BITBUFT* t = (BITBUFT*)outbuf;
        addbits_raw(val, bits, t);
        outbuf = (DWBYTE*) t;
    }

private:
    BITBUFT cur;
    int bits_left;
};


inline
void
Packbits::add0(BITBUFT*& outbuf)
{
#ifdef SHOW
    printf("%x, %x, 0\n", this, outbuf);
#endif
#ifdef RECORD
    dorec(0)
#endif
    if(bits_left >= 1)
    {
        cur <<= 1;
        --bits_left;
    }
    else
    {
        *outbuf++ = int_to_le(cur);
        bits_left = BITBUFSZ - 1;
        cur = 0;
    }
}

inline
void
Packbits::add1(BITBUFT*& outbuf)
{
#ifdef SHOW
    printf("%x, %x, 1\n", this, outbuf);
#endif
#ifdef RECORD
    dorec(1)
#endif
    if(bits_left >= 1)
    {
        cur <<= 1;
        cur |= 1;
        --bits_left;
    }
    else
    {
        *outbuf++ = int_to_le(cur);
        bits_left = BITBUFSZ - 1;
        cur = 1;
    }
}

inline
void
Packbits::addbits_raw(ELTYPE val, int bits, BITBUFT*& outbuf)
{
    val &= (1 << bits) - 1;
#ifdef SHOW
    printf("%d, %d\n", val, bits);
#endif
#ifdef RECORD
    dorec(val)
#endif
    if(bits <= bits_left)
    {
        cur <<= bits;
        cur |= val;
        bits_left -= bits;
        if(bits_left == 0)
        {
            *outbuf++ = int_to_le(cur);
#ifdef SHOW
            printf("%x put %x\n", this, cur);
#endif
            bits_left = BITBUFSZ;
            cur = 0;
        }
    }
    else
    {
        cur <<= bits_left;
        cur |= (val >> (bits - bits_left));
        *outbuf++ = int_to_le(cur);
#ifdef SHOW
        printf("%x put %x\n", this, cur);
#endif
        cur = (val & ((1 << (bits - bits_left)) - 1));
        bits_left = BITBUFSZ - (bits - bits_left);
        if(bits_left == 0)
        {
            *outbuf++ = int_to_le(cur);
#ifdef SHOW
            printf("%x put %x\n", this, cur);
#endif
            bits_left = BITBUFSZ;
            cur = 0;
        }
    }
}


inline
void
Packbits::flush(BITBUFT*& outbuf)
{
    {
        *outbuf++ = int_to_le((cur << bits_left));
#ifdef SHOW
        printf("%x put %x\n", this, cur << bits_left);
#endif
    }
}

inline
void
Packbits::init()
{
    bits_left = BITBUFSZ;
}
#endif

// use this class to unpack a stream produced by Packbits
class Unpackbits
{
public:
    Unpackbits() {
        bits_left = BITBUFSZ;
    }
     void init(BITBUFT*& inbuf);
     void init(DWBYTE*& inbuf) {
        BITBUFT *t = (BITBUFT*)inbuf;
        init(t);
        inbuf = (DWBYTE*)t;
    }
     ELTYPE getbits_raw(int bits, BITBUFT*& inbuf);
     int getbit_raw(BITBUFT*& inbuf);
     ELTYPE getbits_raw(int bits, DWBYTE*& inbuf) {
        BITBUFT *t = (BITBUFT*)inbuf;
        ELTYPE val = getbits_raw(bits, t);
        inbuf = (DWBYTE*)t;
        return val;
    }

protected:
    BITBUFT cur;
    int bits_left;
};
#endif
