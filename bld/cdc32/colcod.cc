
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/colcod.cc 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#include <stdio.h>
#include "colcod.h"
#include "jtcode.h"
#include "chroma.h"
#include "imgmisc.h"
#if __GNUC_MINOR__ >= 7 || defined(__linux__)
#include <netinet/in.h>
#endif
#ifdef __WIN32__
#include "winsock.h"
#endif

DwCoderColor::DwCoderColor()
{
    luma = 0;
    cr = 0;
    cb = 0;
}

DwCoderColor::~DwCoderColor()
{

}

void
DwCoderColor::set_crop_window(int xc, int yc, int wc, int hc)
{
    x = xc;
    y = yc;
    w = wc;
    h = hc;
    if(luma)
        luma->set_crop_window(xc, yc, wc, hc);
    if(cr)
        cr->set_crop_window(xc / 2, yc / 2, wc / 2, hc / 2);
    if(cb)
        cb->set_crop_window(xc / 2, yc / 2, wc / 2, hc / 2);
}

void
DwCoderColor::set_crop(int c)
{
    do_crop = !!c;
    if(luma)
        luma->set_crop(do_crop);
    if(cr) cr->set_crop(do_crop);
    if(cb) cb->set_crop(do_crop);
}

void
DwCoderColor::set_subsample(int s)
{
    do_subsample = s;
    if(luma) luma->set_subsample(s);
    if(cr) cr->set_subsample(s);
    if(cb) cb->set_subsample(s);
}

void
DwCoderColor::set_policy(const char *p)
{
    free(policy);
    policy = strdup(p);
    if(luma) luma->set_policy(p);
    if(cr) cr->set_policy(p);
    if(cb) cb->set_policy(p);
}

void
DwCoderColor::set_pad(int p)
{
    do_pad = !!p;
    if(luma) luma->set_pad(do_pad);
    if(cr) cr->set_pad(do_pad);
    if(cb) cb->set_pad(do_pad);
}

void
DwCoderColor::set_sampling(int s)
{
    sampling = s;
    if(luma) luma->set_sampling(s);
    if(cr) cr->set_sampling(s);
    if(cb) cb->set_sampling(s);
}

void
DwCoderColor::set_special_parms(double a, double b, double c,
                                int d, int e, int f, int g, int h, unsigned int i)
{
    if(luma) luma->set_special_parms(a, b, c, d, e, f, g, h, i);
    if(cr) cr->set_special_parms(a, b, c, d, e, f, g, h, i);
    if(cb) cb->set_special_parms(a, b, c, d, e, f, g, h, i);
}

void outpgm(const char *p, int cnt, gray **img, int cols, int rows);

DWBYTE *
DwCoderColor::code_preprocess(gray **img_l, gray **img_cb, gray **img_cr,
                              int cols, int rows, int is_ref, int& len_out, void *captured_ppm)
{
    if(!busy())
        set_busy(1);
    else
        oopanic("color code busy");

    if(do_crop == 0)
    {
        x = 0;
        y = 0;
        h = rows;
        w = cols;
    }

    if((sampling == SAMPYUV9 || sampling == SAMPYUV12) && img_cb && img_cr)
        codeyuv(img_l, img_cb, img_cr, cols, rows, is_ref, len_out,
                sampling == SAMPYUV9 ? 4 : 2);
    else
    {

        gray **nimg[3];
        gray **nimg2[3];
        int i;
        int r[3], c[3], o_r[3], o_c[3];
        int nplanes = 3;

        if(sampling == SAMP400 || img_cb == 0 || img_cr == 0)
        {
            nplanes = 1;
            sampling = SAMP400;
        }

        int ocols = cols;
        int orows = rows;

        gray **a[3];
        a[0] = img_l;
        a[1] = img_cb;
        a[2] = img_cr;
#if 0
        static int cnt;
        outpgm("il", cnt, img_l, cols, rows);
        outpgm("ib", cnt, img_cb, cols, rows);
        outpgm("ir", cnt, img_cr, cols, rows);
#endif
        // perform up-front croping and subsampling on
        // all three planes identically.
        //
        for(i = 0; i < nplanes; ++i)
        {
            r[i] = orows;
            c[i] = ocols;
            o_r[i] = orows;
            o_c[i] = ocols;
            if(do_subsample)
            {
                nimg[i] = subsample2_1(a[i], &r[i], &c[i]);
                pgm_freearray(a[i], orows);
                o_r[i] = r[i];
                o_c[i] = c[i];
                if(do_crop == 0)
                {
                    w = c[i];
                    h = r[i];
                }
                if(i == 0) // use luma plane as guide
                    captured_ppm = 0;
            }
            else
                nimg[i] = a[i];
//do_pad = 1;
            if((nimg2[i] = crop(nimg[i], &c[i], &r[i], x, y, w, h, do_pad, 1)) != 0)
            {
                pgm_freearray(nimg[i], o_r[i]);
                o_r[i] = r[i];
                o_c[i] = c[i];
                if(i == 0)
                    captured_ppm = 0;
            }
            else
                nimg2[i] = nimg[i];
        }
#if 0
        outpgm("il2", cnt, nimg2[0], c[0], r[0]);
        outpgm("ib2", cnt, nimg2[1], c[1], r[1]);
        outpgm("ir2", cnt, nimg2[2], c[2], r[2]);
#endif
        gray **chr[3];
        gray **chr2[3];

        // now subsample chroma planes depending on the
        // selected 422 or 411 sampling
        for(i = 1; i < nplanes; ++i)
        {
            chr[i] = subsample2_1(nimg2[i], &r[i], &c[i]);
            // note: don't free it here, may need it for display later.
            o_r[i] = r[i];
            o_c[i] = c[i];
            if(sampling == SAMP411)
            {
                chr2[i] = subsample2_1(chr[i], &r[i], &c[i]);
                pgm_freearray(chr[i], o_r[i]);
                chr[i] = chr2[i];
                o_r[i] = r[i];
                o_c[i] = c[i];
            }

            // note: always pad, since we don't want a little part
            // of the picture to be grayscale
            if((chr2[i] = crop(chr[i], &c[i], &r[i], 0, 0, c[i], r[i], 1, 0)) != 0)
            {
                pgm_freearray(chr[i], o_r[i]);
            }
            else
                chr2[i] = chr[i];
        }
#if 0
        outpgm("ib3", cnt, chr2[1], c[1], r[1]);
        outpgm("ir3", cnt, chr2[2], c[2], r[2]);
        ++cnt;
#endif

        if(sampling == SAMP400)
            display_img_2b_coded(nimg2[0], c[0], r[0]);
        else if(captured_ppm)
            display_img_2b_coded((pixel **)captured_ppm, cols, rows);
        else
            display_img_2b_coded(nimg2[0], nimg2[1], nimg2[2], c[0], r[0]);
        for(i = 1; i < nplanes; ++i)
            pgm_freearray(nimg2[i], r[0]); // note: r[0] because we want to free orig image size

        if(!inhibit_coding)
        {
#ifndef CDC32
            // automatically toggle for icuii
            if(c[0] < 320 && r[0] < 240)
                set_policy("better-yet");
            else
                set_policy("better");
#endif
            luma->code_frame(nimg2[0], c[0], r[0], is_ref, policy);
            if(sampling != SAMP400)
            {
                cb->code_frame(chr2[1], c[1], r[1], is_ref, policy);
                cr->code_frame(chr2[2], c[2], r[2], is_ref, policy);
            }
        }
        else
        {
            pgm_freearray(nimg2[0], r[0]);
            if(sampling != SAMP400)
            {
                pgm_freearray(chr2[1], r[1]);
                pgm_freearray(chr2[2], r[2]);
            }
        }
    }

    DWBYTE *b;
    if(!inhibit_coding)
    {
        b = final_package(len_out);
    }
    else
    {
        b = 0;
        len_out = 0;
    }
    set_busy(0);
    return b;
}

// don't do any user defined subsampling, etc.
// just code and send it.
//
void
DwCoderColor::codeyuv(gray **img_l, gray **img_cb, gray **img_cr,
                      int cols, int rows, int is_ref, int& len_out, int subsamp)
{
    cr->sampling = (subsamp == 4 ? SAMP411 : SAMP422);
    cb->sampling = (subsamp == 4 ? SAMP411 : SAMP422);

    gray **a[2];
    int r[2], c[2];
    int i;

    {
        gray **nimg[3];
        gray **nimg2[3];
        int i;
        int ri[3], ci[3], o_r[3], o_c[3];
        int nplanes = 3;

        int ocols = cols;
        int orows = rows;

        gray **a[3];
        a[0] = img_l;
        a[1] = img_cb;
        a[2] = img_cr;
#if 0
        static int cnt;
        outpgm("il", cnt, img_l, cols, rows);
        outpgm("ib", cnt, img_cb, cols, rows);
        outpgm("ir", cnt, img_cr, cols, rows);
#endif
        // perform up-front cropping and subsampling on
        // all three planes identically.
        //
        for(i = 0; i < nplanes; ++i)
        {
            if(i == 0)
            {
                ri[i] = orows;
                ci[i] = ocols;
                o_r[i] = orows;
                o_c[i] = ocols;
            }
            else
            {
                // chroma planes are subsampled already
                ri[i] = orows / subsamp;
                ci[i] = ocols / subsamp;
                o_r[i] = orows / subsamp;
                o_c[i] = ocols / subsamp;
            }
            if(do_subsample)
            {
                nimg[i] = subsample2_1(a[i], &ri[i], &ci[i]);
                pgm_freearray(a[i], orows);
                o_r[i] = ri[i];
                o_c[i] = ci[i];
                if(do_crop == 0)
                {
                    w = ci[i];
                    h = ri[i];
                }
            }
            else
                nimg[i] = a[i];
#if 0
            if((nimg2[i] = crop(nimg[i], &ci[i], &ri[i], x, y, w, h, do_pad, 1)) != 0)
            {
                pgm_freearray(nimg[i], or[i]);
                or[i] = ri[i];
                oc[i] = ci[i];
            }
            else
#endif
                nimg2[i] = nimg[i];
        }
        img_l = nimg2[0];
        img_cb = nimg2[1];
        img_cr = nimg2[2];
        cols = ci[0];
        rows = ri[0];
        r[0] = ri[1];
        r[1] = ri[2];
        c[0] = ci[1];
        c[1] = ci[2];
    }
    // upsample the chroma so we can display it.
    // we wouldn't need to do this if the hardware preview
    // was adequate (sometimes we actually need to see what is
    // being captured.)

    a[0] = img_cb;
    a[1] = img_cr;
    int o_r[2];
    int o_c[2];
    for(i = 0; i < 2; ++i)
    {
        //r[i] = rows / subsamp;
        //c[i] = cols / subsamp;
        o_r[i] = r[i];
        o_c[i] = c[i];
        gray **b = (subsamp == 4 ? upsample2_2 : upsample1_2 )(a[i], &c[i], &r[i]);
        a[i] = b;
    }

    display_img_2b_coded(img_l, a[0], a[1], cols, rows);
    for(i = 0; i < 2; ++i)
        pgm_freearray(a[i], r[i]);

    if(!inhibit_coding)
    {
        // pad out frames
        // note: always pad, since we don't want a little part
        // of the picture to be grayscale

        gray **img_l2;
        int rl = rows;
        int cl = cols;
        if((img_l2 = crop(img_l, &cl, &rl, 0, 0, cols, rows, 1, 0)) != 0)
        {
            pgm_freearray(img_l, rows);
            img_l = img_l2;
        }

        a[0] = img_cb;
        a[1] = img_cr;
        gray **chr2[2];
        for(i = 0; i < 2; ++i)
        {
            //r[i] = rows / subsamp;
            //c[i] = cols / subsamp;
            r[i] = o_r[i];
            c[i] = o_c[i];
            if((chr2[i] = crop(a[i], &c[i], &r[i], 0, 0, c[i], r[i], 1, 0)) != 0)
            {
                pgm_freearray(a[i], o_r[i]);
            }
            else
                chr2[i] = a[i];
        }

#ifndef CDC32
        // automatically toggle for icuii
        if(cl < 320 && rl < 240)
            set_policy("better-yet");
        else
            set_policy("better");
#endif
//printf("luma\n");
        luma->code_frame(img_l, cl, rl, is_ref, policy);
        cb->code_frame(chr2[0], c[0], r[0], is_ref, policy);
        cr->code_frame(chr2[1], c[1], r[1], is_ref, policy);
    }
    else
    {
        pgm_freearray(img_l, rows);
        pgm_freearray(img_cb, rows / subsamp);
        pgm_freearray(img_cr, rows / subsamp);
    }

}
