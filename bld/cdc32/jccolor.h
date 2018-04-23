
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jccolor.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef JCCOLOR_H
#define JCCOLOR_H


#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

void rgb_ycc_start();
void rgb_ycc_convert(
    JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
    JDIMENSION output_row, JDIMENSION num_cols, int num_rows);
#endif
