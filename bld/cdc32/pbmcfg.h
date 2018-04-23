
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/pbmcfg.h 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef PBMCFG_H
#define PBMCFG_H
//#ifdef __cplusplus
//extern "C" {
//#endif
#include "pgm.h"
#include "ppm.h"
#undef min
#undef max
#undef MSDOS

//#ifdef __cplusplus
//};
//#endif
typedef pixel **PPMIMG;
typedef unsigned char **PGMIMG;

#if 0
// put back the min and max we had to get rid of
// above.
#ifdef __MINMAX_DEFINED
#define __MINMAX_DEFINED
template <class T> inline const T _FAR &min( const T _FAR &t1, const T _FAR &t2)
{
    if  (t1 < t2)
        return t1;
    else
        return t2;
}

template <class T> inline const T _FAR &max( const T _FAR &t1, const T _FAR &t2)
{
    if  (t1 > t2)
        return t1;
    else
        return t2;
}
#endif
#endif

#endif
