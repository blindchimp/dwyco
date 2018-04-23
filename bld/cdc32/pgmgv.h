
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/pgmgv.h 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef PGMGV_H
#define PGMGV_H
#include "pbmcfg.h"

void pgm_to_grayview(gray **g, int cols, int rows, grayview *gv);
void pgm_to_grayview(gray **g, int cols, int rows, int gv_id);

#endif
