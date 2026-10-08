#ifndef DWYCO_NO_VIDEO_FROM_PPM
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/acqfile.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "acqfile.h"

// read a color (P6) PPM file. The dummy pixel type arg is what lets readfile
// overload the pbm read functions without a name clash.
//
// Only the color variant is supported: the raw-file capture source has always
// instantiated FileAcquire<pixel>, so a gray (P5) reader was never reachable.

pixel **
readfile(pixel *, FILE *f, int *cols, int *rows)
{
    return ppm_readppm(f, cols, rows);
}
#endif
