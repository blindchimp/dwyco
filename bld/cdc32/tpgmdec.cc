
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tpgmdec.cc 1.23 1999/01/10 16:09:41 dwight Checkpoint $
 */
#include "tpgmdec.h"
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"
#include "gvchild.h"
#include "netcod.h"
#include "ppmcv.h"

void (*TPGMMSWDecoderColor::display_info_in_titlebar_callback)(int, const char *);
void (*TPGMMSWDecoder::display_info_in_titlebar_callback)(int, const char *);
void (*TPGMMSWDecoderColor::resize_video_window_callback)(int id, int c, int r);
void (*TPGMMSWDecoder::resize_video_window_callback)(int id, int c, int r);

TPGMDecoder::~TPGMDecoder()
{
    destr_fun();
}

void
TPGMDecoder::convert(MAT m, void *app_img, int i, int j, QTAB *)
{
    gray **dimg;
    if(app_img != 0)
        dimg = (gray **)app_img;
    else if(ref_img == 0)
    {
        oopanic("bad ref img");
    }
    else
        dimg = (gray **)ref_img;

    for(int k = 0; k < MSZ; ++k)
        for(int l = 0; l < MSZ; ++l)
        {
            int t = m[k][l] + BIAS;
            if(t < 0) t = 0;
            else if (t > 255) t = 255;
            dimg[i + k][j + l] = t;
        }
}

void *
TPGMDecoder::alloc_appimg(int blkw, int blkh)
{
    return pgm_allocarray(blkw * MSZ, blkh * MSZ);
}

void
TPGMDecoder::dealloc_refimg()
{
    if(ref_img)
    {
        pgm_freearray((gray **)ref_img, ri_rows);
        ref_img = 0;
    }
}

#include "pgmgv.h"
#include "grayview.h"
void
TPGMMSWDecoder::convert(MAT m, void *app_img, int i, int j, QTAB *)
{
    gray **dimg;
    if(app_img != 0)
        dimg = (gray **)app_img;
    else if(ref_img == 0)
    {
        oopanic("bad ref img2");
    }
    else
        dimg = (gray **)ref_img;

    for(int k = 0; k < MSZ; ++k)
        for(int l = 0; l < MSZ; ++l)
        {
            int t = m[k][l] + BIAS;
            if(t < 10) t = 10;
            else if (t > 245) t = 245;
            dimg[i + k][j + l] = t;
        }
}

void
TPGMMSWDecoder::display_decoded(void *v, int cols, int rows)
{
    gray **g = (gray **)v;
    if(resize_video_window_callback)
        (*resize_video_window_callback)(gv_id, cols, rows);
    pgm_to_grayview(g, cols, rows, gv_id);
}

int
TPGMMSWDecoder::finish_line()
{
    return 1;
}

int
TPGMMSWDecoder::finish_block()
{
    return 1;
}

void
TPGMMSWDecoder::display_info(const char *str)
{
    if(display_info_in_titlebar_callback)
        (*display_info_in_titlebar_callback)(gv_id, str);
}

void
TPGMMSWDCTDecoder::convert(MAT m, void *app_img, int i, int j, QTAB *qp)
{
    gray **dimg = 0;
    if(app_img != 0)
        dimg = (gray **)app_img;
    else if(ref_img == 0)
    {
        oopanic("bad ref img3");
    }
    else
        dimg = (gray **)ref_img;

    jpeg_idct_islow((JCOEFPTR)&m[0][0], &dimg[i], j, qp, 1);

}

void
TPGMMSWDecoderColor::display_info(const char *str)
{
    if(display_info_in_titlebar_callback)
        (*display_info_in_titlebar_callback)(gv_id, str);
}

void
TPGMMSWDecoderColor::display_decoded(void *p, int cols, int rows)
{
    pixel **img = (pixel **)p;
    if(resize_video_window_callback)
        (*resize_video_window_callback)(gv_id, cols, rows);
    ppm_to_colorview(img, cols, rows, gv_id);
}

void
TPGMDCTDecoder::convert(MAT m, void *app_img, int i, int j, QTAB *qp)
{
    gray **dimg;
    if(app_img != 0)
        dimg = (gray **)app_img;
    else if(ref_img == 0)
    {
        oopanic("bad reg img4");
    }
    else
        dimg = (gray **)ref_img;

    jpeg_idct_islow((JCOEFPTR)m, &dimg[i], j, qp, 1);
}



