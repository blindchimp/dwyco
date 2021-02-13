
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/codec.cc 1.8 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "codec.h"
#include "imgmisc.h"
[[noreturn]] void oopanic(const char *);

int
DwCodec::busy()
{
    return is_busy;
}

void
DwCodec::set_busy(int b)
{
    is_busy = !!b;
}

DwCoder::DwCoder()
{
    do_crop = 0;
    x = 0;
    y = 0;
    w = 0;
    h = 0;
    do_subsample = 0;
    do_pad = 0;
    policy = strdup("default");
    sampling = SAMP411;
    inhibit_coding = 0;
}

DwCoder::~DwCoder()
{
    free(policy);
}

void
DwCoder::set_crop_window(int xc, int yc, int wc, int hc)
{
    x = xc;
    y = yc;
    w = wc;
    h = hc;
}

void
DwCoder::set_crop(int c)
{
    do_crop = !!c;
}

void
DwCoder::set_subsample(int s)
{
    do_subsample = s;
}

void
DwCoder::set_policy(const char *p)
{
    free(policy);
    policy = strdup(p);
}

void
DwCoder::set_pad(int p)
{
    do_pad = !!p;
}

void
DwCoder::set_sampling(int s)
{
    sampling = s;
}

int
DwCoder::get_sampling()
{
    return sampling;
}

DWBYTE *
DwCoder::code_preprocess(gray **img, int cols, int rows, int is_ref, int& len_out)
{
    if(!busy())
        set_busy(1);
    else
        oopanic("busy codec");
    gray **nimg;
    gray **nimg2;

    int ocols = cols;
    int orows = rows;
    if(do_subsample)
    {
        nimg = subsample2(img, &rows, &cols);
        pgm_freearray(img, orows);
        orows = rows;
        ocols = cols;
    }
    else
        nimg = img;
    if(do_crop == 0)
    {
        x = 0;
        y = 0;
        h = rows;
        w = cols;
    }
    if((nimg2 = crop(nimg, &cols, &rows, x, y, w, h, do_pad, 0)) != 0)
    {
        pgm_freearray(nimg, orows);
        orows = rows;
        ocols = cols;
    }
    else
        nimg2 = nimg;

    display_img_2b_coded(nimg2, cols, rows);

    DWBYTE *b;
    if(!inhibit_coding)
    {
#ifndef CDC32
        // automatically toggle for icuii
        if(cols < 320 && rows < 240)
            set_policy("better-yet");
        else
            set_policy("better");
#endif
        code_frame(nimg2, cols, rows, is_ref, policy);
        b = final_package(len_out);
    }
    else
    {
        b = 0;
        len_out = 0;
        pgm_freearray(nimg2, rows);
    }
    set_busy(0);
    return b;
}

// decoders

DwDecoder::DwDecoder()
{
    do_upsample = 0;
    decode_color = 1;
    force_gray = 0;
    tweak = 0;
    sampling = -1;
}

void
DwDecoder::decode_postprocess(DWBYTE *buf, int len)
{
    if(!busy())
        set_busy(1);
    else
        oopanic("busy decoder");
    void *vimg = 0;
    int cols;
    int rows;
    decode_from_stream(buf, len, vimg, cols, rows);
    if(!vimg)
        return;
    gray **g = (gray **)vimg;
    gray **g2 = 0;

    switch(do_upsample)
    {
    case 0:
        break;
    case 1:
        g2 = upsample0_2(g, &cols, &rows);
        g = g2;
        break;
    case 2:
        g2 = upsample1_2(g, &cols, &rows);
        g = g2;
        break;
    default:
        oopanic("bad decode upsample");
    }

    display_decoded(g, cols, rows);
    if(g2)
        pgm_freearray(g2, rows);
    set_busy(0);
}

void
DwDecoder::set_upsample(int u)
{
    do_upsample = u;
}

void
DwDecoder::set_tweak(unsigned int u)
{
    tweak = u;
}

int
DwDecoder::get_sampling()
{
    return sampling;
}

void
DwDecoder::set_sampling(int s)
{
    sampling = s;
}

