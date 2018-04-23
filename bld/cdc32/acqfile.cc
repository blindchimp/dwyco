
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

// these are overloaded functions that have a dummy type
// arg to get around the naming problem in the pbm read
// functions.

gray **
readfile(gray *, FILE *f, int *cols, int *rows, gray *maxval)
{
    return pgm_readpgm(f, cols, rows, maxval);
}

pixel **
readfile(pixel *, FILE *f, int *cols, int *rows, gray *maxval)
{
    return ppm_readppm(f, cols, rows, maxval);
}
