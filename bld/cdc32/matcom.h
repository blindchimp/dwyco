
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/matcom.h 1.8 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef MATCOM_H
#define MATCOM_H

#define MSZ 8
#define L2MSZ 3
#define STRUNC ((1 << L2MSZ) - 1)
#define MAXVEC 1024

typedef unsigned short dwUINT16;
typedef unsigned long dwUINT32min;
typedef int dwINT32;
typedef unsigned int dwUINT32;

typedef int ELTYPE;
typedef unsigned char DWBYTE;
typedef unsigned short DWUSHORT;
typedef dwUINT32 BITBUFT;
typedef ELTYPE MAT4_4[4][4];
typedef ELTYPE MAT8_8[8][8];
typedef ELTYPE MAT16_16[16][16];
typedef ELTYPE MAT32_32[32][32];
typedef ELTYPE MAT64_64[64][64];
typedef ELTYPE VEC16[16];
typedef MAT8_8 MAT;
typedef ELTYPE VEC[MSZ];

#define BLKR (480 / MSZ)
#define BLKC (720 / MSZ)
typedef MAT XFRM[1][1];

#define BIAS 128

typedef double BLKFEA[BLKR][BLKC];

#endif
