
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jdcolor.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef JDDOLOR_H
#define JDDOLOR_H

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

void build_ycc_rgb_table ();
void ycc_rgb_convert (
    JSAMPIMAGE input_buf, JDIMENSION input_row,
    JSAMPARRAY output_buf, JDIMENSION num_cols, int num_rows);
#endif
