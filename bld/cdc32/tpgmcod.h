#ifdef DWYCO_DCT_CODER
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tpgmcod.h 1.6 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef TPGMCOD_H
#define TPGMCOD_H
#include "pbmcfg.h"
#include "tcode.h"
#include "jtcode.h"
#include "jcolcod.h"
#include "colcod.h"
#include "grayview.h"

class TMSWCoderColor :  public JPEGCoderColor
{
public:

    void display_img_2b_coded(gray **g, int cols, int rows);
    void display_img_2b_coded(pixel **g, int cols, int rows);
    void display_img_2b_coded(gray **y, gray **cb, gray **cr, int cols, int rows);
    void display_info(const char *);
    static void (*resize_video_window_callback)(int id, int cols, int rows);
};

#endif
#endif

