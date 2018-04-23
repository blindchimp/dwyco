
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/macs.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef MACS_H
#define MACS_H

#define iabs(x) if((x) < 0) (x) = (-(x));
#define sgn(x) ((x) < 0 ? -1 : ((x) == 0 ? 0 : 1))
#define qabs(x) ((x) < 0 ? -(x) : (x))
#define dwround(x) ((int)((x) + .5))
#define dw_min(x, y) (((x) > (y)) ? (y) : (x))
#define dw_max(x, y) (((x) < (y)) ? (y) : (x))

#endif
