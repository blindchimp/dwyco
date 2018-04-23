
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/mo.h 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef MO_H
#define MO_H
#include "pbmcfg.h"

#define MV_ZERO 4

double bm_motion_estimate(gray **last_frame, int *tmp, int fr, int fc,
                          int& vi, int& vj);
double motion_estimate(gray **last_frame, gray **frame, int fr, int fc,
                       int& vi, int& vj);
double lbm_motion_estimate8x8(gray **last_frame, int *frame, int fr, int fc, int& vi, int &vj);

#endif
