
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tpgmcod.cc 1.15 1999/01/10 16:09:40 dwight Checkpoint $
 */
#include "gvchild.h"
#include "tpgmcod.h"
#include "pgmgv.h"
#include "tpgmdec.h"
#include "netcod.h"
#include "jdcolor.h"
#include "pbmcfg.h"
#include "ppmcv.h"
#include "dwrtlog.h"

void (*TMSWCoderColor::resize_video_window_callback)(int id, int cols, int rows);

void
TMSWCoderColor::display_img_2b_coded(gray **y, gray **cb, gray **cr, int cols, int rows)
{
    DwVec<int> tmp = gvs;
    tmp.append(gv_id);
    int n = tmp.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int gg = tmp[i];
        if(gg <= 0)
            continue;
        // XXX add grayview is iconic test...
        PPMIMG p = ppm_allocarray(cols, rows);
        JSAMPIMAGE a = new JSAMPARRAY[3];
        a[0] = y;
        a[1] = cb;
        a[2] = cr;
        ycc_rgb_convert(a, 0, (unsigned char **)p, cols, rows);
        if(resize_video_window_callback)
            (*resize_video_window_callback)(gg, cols, rows);
        ppm_to_colorview(p, cols, rows, gg);
        ppm_freearray(p, rows);
        delete [] a;
    }
}

void
TMSWCoderColor::display_img_2b_coded(gray **g, int cols, int rows)
{
    DwVec<int> tmp = gvs;
    tmp.append(gv_id);
    int n = tmp.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int gg = tmp[i];
        if(resize_video_window_callback)
            (*resize_video_window_callback)(gg, cols, rows);
        pgm_to_grayview(g, cols, rows, gg);
    }
}

void
TMSWCoderColor::display_img_2b_coded(pixel **g, int cols, int rows)
{
    DwVec<int> tmp = gvs;
    tmp.append(gv_id);
    int n = tmp.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int gg = tmp[i];
        if(gg <= 0)
            continue;
        if(resize_video_window_callback)
            (*resize_video_window_callback)(gg, cols, rows);
        ppm_to_colorview(g, cols, rows, gg);
    }
}

static void
display_info(const char *str, int gv_id)
{
    set_caption_by_id(gv_id, str);
}

void
TMSWCoderColor::display_info(const char *str)
{
    ::display_info(str, gv_id);
}
