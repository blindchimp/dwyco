
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/coldec.cc 1.12 1999/01/10 16:09:29 dwight Checkpoint $
 */
#include <stdio.h>
#include "pbmcfg.h"
#include "coldec.h"
#include "jtdecode.h"
#include "dchroma.h"
#include "imgmisc.h"
#include "jdcolor.h"
#include "tpgmdec.h"
#include "dwrtlog.h"
void oopanic(const char *);

DwDecoderColor::DwDecoderColor()
{
#if 0
    luma = new TPGMMSWDCTDecoder;
    cb = new JPEGTDecoderChroma(0, (TPGMMSWDCTDecoder *)luma);
    cr = new JPEGTDecoderChroma(1, (TPGMMSWDCTDecoder *)luma);
#endif
    luma = 0;
    cb = 0;
    cr = 0;
}

DwDecoderColor::~DwDecoderColor()
{
#if 0
    delete cb;
    delete cr;
    delete luma;
#endif
}

#if 0
void
outpgm(const char *p, int cnt, gray **img, int cols, int rows)
{
    return;
    char s[255];
    sprintf(s, "%s_%03d.pgm", p, cnt);
    FILE *f = pm_openw(s);
    pgm_writepgm(f, img, cols, rows, 255, 0);
    fclose(f);
}
#endif

void
DwDecoderColor::decode_from_stream(DWBYTE*& buf, int& len, void *&vimg, int &cols, int& rows)
{
    luma->set_tweak(tweak);
    if(cb) cb->set_tweak(tweak);
    if(cr) cr->set_tweak(tweak);
    vimg = 0;

//printf("luma\n");
    luma->decode_from_stream(buf, len, vimg, cols, rows);
    int ocols = cols;
    int orows = rows;
    gray **gl = (gray **)vimg;
    if(force_gray || !decode_color || /*sampling == SAMP400 ||*/ luma->get_sampling() == SAMP400)
    {
        // shortcut here, there is only a luma plane
        // to be had...
        return;
    }
    vimg = 0;
//printf("blue\n");
    cb->decode_from_stream(buf, len, vimg, cols, rows);
    if(cb->get_sampling() == SAMP400)
    {
        // shortcut here, there is only a luma plane
        // to be had... (ie, chroma plane doesn't exist
        // in this stream.
        return;
    }
    gray **gcb = (gray **)vimg;
    vimg = 0;
    int occols = cols;
    int ocrows = rows;
    // note: cols and rows reflect subsampled chroma
    // that comes back from decoder
//printf("red\n");
    cr->decode_from_stream(buf, len, vimg, cols, rows);
    gray **gcr = (gray **)vimg;

    gray **gcr2 = 0;
    if(cr->get_sampling() == SAMP422)
    {
        gcr2 = upsample1_2(gcr, &cols, &rows);
    }
    else if(cr->get_sampling() == SAMP411)
    {
        gcr2 = upsample2_2(gcr, &cols, &rows);
    }
    else
        oopanic("bad chroma sampling");

    rows = ocrows;
    cols = occols;

    gray **gcb2 = 0;
    if(cb->get_sampling() == SAMP422)
    {
        gcb2 = upsample1_2(gcb, &cols, &rows);
    }
    else if(cb->get_sampling() == SAMP411)
    {
        gcb2 = upsample2_2(gcb, &cols, &rows);
    }
    else
        oopanic("bad chroma sampling2");

    // note: upsampled chroma may be different in
    // size than original luma, but the extra
    // can be ignored.

    // got all the components, now merge them into
    // a ppm-like rgb thing
    PPMIMG p = ppm_allocarray(ocols, orows);
    JSAMPIMAGE a = new JSAMPARRAY[3];
    a[0] = gl;
    a[1] = gcb2;
    a[2] = gcr2;

    ycc_rgb_convert(a, 0, (unsigned char **)p, ocols, orows);
    vimg = p;
    cols = ocols;
    rows = orows;

    delete [] a;
    pgm_freearray(gcr2, rows);
    pgm_freearray(gcb2, rows);

}

void
DwDecoderColor::decode_postprocess(DWBYTE *buf, int len)
{
    if(!busy())
        set_busy(1);
    else
        oopanic("decode color busy");
    luma->set_tweak(tweak);
    if(cb) cb->set_tweak(tweak);
    if(cr) cr->set_tweak(tweak);
    void *vimg = 0;
    int cols;
    int rows;
//printf("luma\n");
    luma->decode_from_stream(buf, len, vimg, cols, rows);
    int ocols = cols;
    int orows = rows;
    gray **gl = (gray **)vimg;
    if(force_gray || !decode_color || /*sampling == SAMP400 ||*/ luma->get_sampling() == SAMP400)
    {
        // shortcut here, there is only a luma plane
        // to be had...
        luma->gv_id = gv_id;
        luma->display_decoded(vimg, cols, rows);
        set_busy(0);
        return;
    }
    vimg = 0;
//printf("blue\n");
    cb->decode_from_stream(buf, len, vimg, cols, rows);
    if(cb->get_sampling() == SAMP400)
    {
        // shortcut here, there is only a luma plane
        // to be had... (ie, chroma plane doesn't exist
        // in this stream.
        luma->gv_id = gv_id;
        luma->display_decoded(gl, cols, rows);
        set_busy(0);
        return;
    }
    gray **gcb = (gray **)vimg;
    vimg = 0;
    int occols = cols;
    int ocrows = rows;
    // note: cols and rows reflect subsampled chroma
    // that comes back from decoder
//printf("red\n");
    cr->decode_from_stream(buf, len, vimg, cols, rows);
    gray **gcr = (gray **)vimg;
    static int cnt;
    //outpgm("r1", cnt, gcr, cols, rows);
    gray **gcr2 = 0;
    if(cr->get_sampling() == SAMP422)
    {
        gcr2 = upsample1_2(gcr, &cols, &rows);
    }
    else if(cr->get_sampling() == SAMP411)
    {
        gcr2 = upsample2_2(gcr, &cols, &rows);
    }
    else
        oopanic("bad chroma sampling");
    //outpgm("r2", cnt, gcr2, cols, rows);
    rows = ocrows;
    cols = occols;
    //outpgm("b1", cnt, gcb, cols, rows);
    gray **gcb2 = 0;
    if(cb->get_sampling() == SAMP422)
    {
        gcb2 = upsample1_2(gcb, &cols, &rows);
    }
    else if(cb->get_sampling() == SAMP411)
    {
        gcb2 = upsample2_2(gcb, &cols, &rows);
    }
    else
        oopanic("bad chroma sampling2");
    //outpgm("b2", cnt, gcb2, cols, rows);
    ++cnt;
    // note: upsampled chroma may be different in
    // size than original luma, but the extra
    // can be ignored.

    // got all the components, now merge them into
    // a ppm-like rgb thing
    PPMIMG p = ppm_allocarray(ocols, orows);
    JSAMPIMAGE a = new JSAMPARRAY[3];
    a[0] = gl;
    a[1] = gcb2;
    a[2] = gcr2;
    //memset(&gcr2[0][0], 0, ocols * orows);
    ycc_rgb_convert(a, 0, (unsigned char **)p, ocols, orows);
    display_decoded(p, ocols, orows);
    GRTLOG("display decoded", 0, 0);
    ppm_freearray(p, orows);
    delete [] a;
    pgm_freearray(gcr2, rows);
    pgm_freearray(gcb2, rows);
    set_busy(0);
}
