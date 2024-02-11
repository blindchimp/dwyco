
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma option -w-sig
#include "vidaq.h"
#include "pbmcfg.h"
#include "jccolor.h"
#include "vidcvt.h"
//#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#include "stb_image_resize.h"

#define MASKFUN(x) (((((int)(x) + samp_off) ^ xor_mask) & and_mask) | or_mask)
//
// when converting to grayscale, we assume the order is
// BGR (as microsoft crap is) for 24 bits samples 16 bit
// samples, because some boards seem to produce those
// orderings.
//


VidConvert::VidConvert()
{
    autoconfig = 0;
    ccols = 0;
    crows = 0;
    cdepth = 0;
    style = AQ_YUV9;
    upside_down = 0;
    and_mask = 0xff;
    xor_mask = 0;
    or_mask = 0;
    samp_off = 0;
    swap_rb = 0;
    force_gray = 0;
    swap_uv = 0;
    palinfo = 0;
    compression = 0;
    is_palette = 0;
    palette = 0;
}



void
VidConvert::set_format(int s)
{
    style = (style & ~AQ_FORMAT) | (s & AQ_FORMAT);
}

void
VidConvert::set_style(int s)
{
    style = s;
}

void
VidConvert::add_style(int s)
{
    style |= s;
}

void
VidConvert::remove_style(int s)
{
    style &= ~s;
}

void
VidConvert::set_upside_down(int s)
{
    upside_down = !!s;
}


VidConvert::~VidConvert()
{
    if(palinfo)
        free(palinfo);
}


static gray Colgray15[32768U];
static gray  Colgray15_s[32768U];
static int  m30[256];
static int  m59[256];
static int  m11[256];

static void
gen_table()
{
    int i;
    for(i = 0; i < 32; ++i)
        for(int j = 0; j < 32; ++j)
            for(int k = 0; k < 32; ++k)
            {
                int idx = k | (j << 5) | (i << 10);
                int idx2 = (k << 10) | (j << 5) | i;
                Colgray15[idx] = ((30 * i + 59 * j + 11 * k) * 8) / 100;
                Colgray15_s[idx2] = ((30 * k + 59 * j + 11 * i) * 8) / 100;
            }
    // note: multiplied by 1.28 so that >>7 works to divide
    // by 128...
    for(i = 0; i < 256; ++i)
    {
        m30[i] = i * 38;
        m59[i] = i * 76;
        m11[i] = i * 14;
    }
}


void *
VidConvert::convert_data(unsigned char *buf, int buflen, int& cols, int& rows,
                         void*& y, void*& cb, void*& cr, int& fmt, int no_ycbcr)
{
    cr = 0;
    cb = 0;
    y = 0;
    fmt = 0;
    ccols = cols;
    crows = rows;

    static int been_here;
    if(!been_here)
    {
        gen_table();
        rgb_ycc_start();
        been_here = 1;
    }
    if(force_gray)
        remove_style(AQ_COLOR);
    // convert buffer from whatever it is (seems to
    // be pretty random what various vendors provide)
    // to 8 bit pgm for now...
    int bytes_got = buflen;
    if(autoconfig)
    {
        // attempt to figure it out based on size first

        if(bytes_got == ccols * crows * 3)
            set_format(AQ_RGB24);
        // voodoo numbers: mmiofourcc compression numbers for UYVY and YUY2
        else if(bytes_got == ccols * crows * 2 &&
                (compression != 0x59565955 && compression != 0x32595559))
            set_format(AQ_RGB555);
        else if(bytes_got == ccols * crows * 2 && compression == 0x59565955)
        {
            set_format(AQ_UYVY);
            upside_down = 1;
            swap_uv = 0;
        }
        else if(bytes_got == ccols * crows * 2 && compression == 0x32595559)
        {
            set_format(AQ_YUY2);
            upside_down = 1;
            swap_uv = 0;
        }
        else if(bytes_got == ccols * crows || bytes_got == ccols * crows / 2)
            set_format(AQ_INDEX);
        else if(bytes_got == ccols * crows + 2 * ccols * crows / 4)
        {
            set_format(AQ_YUV12);
            upside_down = 1;
            swap_uv = 1;
        }
        else if(bytes_got == ccols * crows + 2 * ccols * crows / 16)
        {
            set_format(AQ_YUV9);
            upside_down = 1;
            swap_uv = 0;
        }
        else if(cdepth == 16)
        {
            set_format(AQ_RGB555);
        }
        else if(cdepth == 24)
        {
            set_format(AQ_RGB24);
        }
        else if(cdepth == 8 || cdepth == 4)
        {
            // palette member already setup at init time
            set_format(AQ_INDEX);
        }
        else
        {
            // some oddball format
            // pick yuv9 just in case
            set_format(AQ_YUV9);
        }
    }

    gray **g = 0;
    pixel **p = 0;

    // yuv9 returns 3 gray planes instead of a PPM.
    // this gives the app the opportunity to avoid
    // a color conversion.
    if((style & AQ_COLOR) && !(style & (AQ_YUV9|AQ_YUV12|AQ_UYVY|AQ_YUY2|AQ_NV21)))
        p = ppm_allocarray(ccols, crows);
    else
        g = pgm_allocarray(ccols, crows);

    unsigned short *b = (unsigned short *)buf;

    unsigned char *c = (unsigned char *)b;
    long bytes_needed = (long)ccols * (long)crows * 3L;
    if(style & AQ_YUV9)
    {
        bytes_needed = (long)ccols * (long)crows;
        if(style & AQ_COLOR)
            bytes_needed += 2 * ((ccols * crows) / 16);
    }
    else if(style & (AQ_YUV12|AQ_NV21))
    {
        bytes_needed = (long)ccols * (long)crows;
        if(style & AQ_COLOR)
            bytes_needed += 2 * ((ccols * crows) / 4);
    }
    else if(style & AQ_RGB555)
        bytes_needed = (long)ccols * (long)crows * 2L;
    else if(style & AQ_RGB24)
        bytes_needed = (long)ccols * (long)crows * 3L;
    else if(style & AQ_INDEX)
    {
        if(cdepth == 8)
            bytes_needed = (long)ccols * (long)crows;
        else if(cdepth == 4)
            bytes_needed = ((long)ccols * (long)crows) / 2L; // assumed packed

    }
    else if(style & (AQ_UYVY|AQ_YUY2))
    {
        bytes_needed = ccols * crows + 2 * (ccols / 2) * crows;
    }

    if(buflen < bytes_needed)
    {
	// not enough data available to do the requested conversion
	if(p) memset(&p[0][0], 0, ccols * crows * 3);
	if(g) memset(&g[0][0], 0, ccols * crows);
    }
    else if(style & (AQ_YUV9|AQ_YUV12|AQ_NV21))
    {
        // not upside down, all luma first
        // usually this will be upside down because
        // even tho the YUV formats come in rightside-up
        // if we don't flip it here, we end up doing it
        // before display, since all the fucker-microsoft
        // RGB bitmap stuff is upside down.
        if((autoconfig && !upside_down) || (!autoconfig && upside_down))
        {
            if(and_mask == 0xff && or_mask == 0 && xor_mask == 0 && samp_off == 0)
            {
                memcpy(&g[0][0], c, ccols * crows);
                c += ccols * crows;
            }
            else
            {
                for(int i = 0; i < crows; ++i)
                {
                    gray *gr = &g[i][0];
                    for(int j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                    }
                }
            }
        }
        else
        {
            if(and_mask == 0xff && or_mask == 0 && xor_mask == 0 && samp_off == 0)
            {
                for(int i = crows - 1; i >= 0; --i)
                {
                    gray *gr;
                    gr = &g[i][0];
                    memcpy(gr, c, ccols);
                    c += ccols;
                }
            }
            else
            {
                for(int i = crows - 1; i >= 0; --i)
                {
                    gray *gr = &g[i][0];
                    for(int j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                    }
                }
            }
        }
        // c points to first chroma plane, which is subsampled
        // in both directions from the luma plane.
        // it's the app's job to know the format and what
        // to do about it.
        if(style & AQ_COLOR)
        {
            if(style & AQ_NV21)
            {
                get_interleaved_chroma_planes(c, cr, cb, 2);
                if(!autoconfig && swap_uv)
                {
                    void *tmp = cr;
                    cr = cb;
                    cb = tmp;
                }
#if 0
                {
                    gray **out = pgm_allocarray(480 / 2, 360 / 2);
                    gray **crg = (gray **)cr;
                    stbir_resize_uint8(&crg[0][0], ccols / 2, crows / 2, 0, &out[0][0], 480 / 2, 360 / 2, 0, 1);
                    pgm_freearray(crg, crows / 2);
                    cr = out;

                    out = pgm_allocarray(480 / 2, 360 / 2);
                    gray **cbg = (gray **)cb;
                    stbir_resize_uint8(&cbg[0][0], ccols / 2, crows / 2, 0, &out[0][0], 480 / 2, 360 / 2, 0, 1);
                    pgm_freearray(cbg, crows / 2);
                    cb = out;

                    out = pgm_allocarray(480, 360);
                    stbir_resize_uint8(&g[0][0], ccols, crows, 0, &out[0][0], 480, 360, 0, 1);
                    pgm_freearray(g, crows);
                    g = out;
                    ccols = 480;
                    crows = 360;
                }
#endif
            }
            else
            {
                cr = get_chroma_plane(c, (style & AQ_YUV12) ? 2 : 4);
                cb = get_chroma_plane(c, (style & AQ_YUV12) ? 2 : 4);
                if((autoconfig && swap_uv) ||
                        (!autoconfig && (style & AQ_YUV12) && !swap_uv) ||
                        (!autoconfig && (style & AQ_YUV9) && swap_uv))
                {
                    void *tmp = cr;
                    cr = cb;
                    cb = tmp;
                }
            }
        }
    }
    else if((style & AQ_UYVY) || (style & AQ_YUY2))
    {
        gray **cbo = pgm_allocarray(ccols / 2, crows / 2);
        gray **cro = pgm_allocarray(ccols / 2, crows / 2);
        unpack_to_planar(c, g, cbo, cro);
        if(!autoconfig && swap_uv)
        {
            cr = cbo;
            cb = cro;
        }
        else
        {
            cb = cbo;
            cr = cro;
        }
    }
    else if((style & AQ_ONE_PLANE) && !(style & AQ_COLOR))
        grab_one_plane(c, g);
    else if((style & AQ_ALL_PLANES) && !(style & AQ_COLOR))
        rgb_to_gray(c, g);
    else if(style & AQ_COLOR)
    {
        if(no_ycbcr)
        {
            unpack_to_ppm(c, p, 0, 0, 0, 1);
            y = 0;
            cb = 0;
            cr = 0;
        }
        else
        {
            gray **yo = pgm_allocarray(ccols, crows);
            gray **cbo = pgm_allocarray(ccols, crows);
            gray **cro = pgm_allocarray(ccols, crows);
            unpack_to_ppm(c, p, yo, cbo, cro, no_ycbcr);
            // note: no swap uv here since the input was rgb
            y = yo;
            cb = cbo;
            cr = cro;
        }
    }

    cols = ccols;
    rows = crows;
    fmt = style;
    if(style & (AQ_YUV9|AQ_YUV12|AQ_UYVY|AQ_YUY2|AQ_NV21))
        return g;
    if(style & AQ_COLOR)
        return p;
    return g;
}

void
VidConvert::grab_one_plane(unsigned char *buf, gray **g)
{
    // grab one plane (set saturation to 0)
    unsigned short *b = (unsigned short *)buf;
    unsigned char *c = (unsigned char *)buf;
    int clroff = 0;
    // assume c is pointing to a blue sample, unless
    // swapping is spec'ed for red&blue
    switch(style & (AQ_PLANES | AQ_DEPTH))
    {
    case AQ_BLUE|AQ_RGB24:
        if(swap_rb)
            c += 2;
        break;
    case AQ_GREEN|AQ_RGB24:
        ++c;
        break;
    case AQ_RED|AQ_RGB24:
        if(!swap_rb)
            c += 2;
        break;
    // note: this is bad, assumes layout of RGBQUAD struct
    // is packed bytes in bgr order. note: technically, the
    // swap item shouldn't be used here since the
    // order of the rgbquad struct is spec'ed, but for
    // flexibility, we do it anyway.
    case AQ_RED|AQ_INDEX:
        if(!swap_rb)
            clroff = 2;
        break;
    case AQ_GREEN|AQ_INDEX:
        clroff = 1;
        break;
    case AQ_BLUE|AQ_INDEX:
        if(swap_rb)
            clroff = 2;
        break;
    }
    if((style & AQ_INDEX) && !palette)
        return;

    int do_mask = (and_mask != 0xff || or_mask != 0 || xor_mask != 0 || samp_off != 0);

    int new_style = style;
    if(swap_rb)
    {
        switch(style & (AQ_PLANES | AQ_DEPTH))
        {
        case AQ_RED|AQ_RGB555:
            new_style = (style & ~(AQ_PLANES|AQ_DEPTH)) | (AQ_BLUE|AQ_RGB555);
            break;
        case AQ_BLUE|AQ_RGB555:
            new_style = (style & ~(AQ_PLANES|AQ_DEPTH)) | (AQ_RED|AQ_RGB555);
            break;
        }
    }

    for(int i = 0; i < crows; ++i)
    {
        gray *gr;
        int j;
        if(upside_down)
            gr = &g[crows - i - 1][0];
        else
            gr = &g[i][0];
        switch(new_style & (AQ_PLANES | AQ_DEPTH))
        {
        default:
            // hmm, masks aren't set right, just don't do
            // anything in order to avoid a possible crash.
            return;
        case AQ_BLUE|AQ_RGB555:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = (*b++ & 0x1f) << 3;
                    *gr++ = (*b++ & 0x1f) << 3;
                    *gr++ = (*b++ & 0x1f) << 3;
                    *gr++ = (*b++ & 0x1f) << 3;
                }
            }
            break;

        case AQ_GREEN|AQ_RGB555:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                }
            }
            break;

        case AQ_RED|AQ_RGB555:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = (*b++ >> 7) & 0xf8;
                    *gr++ = (*b++ >> 7) & 0xf8;
                    *gr++ = (*b++ >> 7) & 0xf8;
                    *gr++ = (*b++ >> 7) & 0xf8;
                }
            }
            break;
        case AQ_BLUE|AQ_RGB24:
        // FALL THROUGH
        case AQ_GREEN|AQ_RGB24:
        // FALL THROUGH
        case AQ_RED|AQ_RGB24:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN(c[0]);
                    *gr++ = MASKFUN(c[3]);
                    *gr++ = MASKFUN(c[6]);
                    *gr++ = MASKFUN(c[9]);
                    c += 12;
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = c[0];
                    *gr++ = c[3];
                    *gr++ = c[6];
                    *gr++ = c[9];
                    c += 12;
                }
            }
            break;

        case AQ_BLUE|AQ_INDEX:
        case AQ_GREEN|AQ_INDEX:
        case AQ_RED|AQ_INDEX:
            if(do_mask)
            {
                if(cdepth == 8)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[0]] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[1]] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[2]] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[3]] + clroff));
                        c += 4;
                    }
                }
                else if(cdepth == 4)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[0] >> 4] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[0] & 0xf] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[1] >> 4] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[1] & 0xf] + clroff));
                        c += 2;
                    }
                }
            }
            else
            {
                if(cdepth == 8)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = *((unsigned char *)&palette[c[0]] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[1]] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[2]] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[3]] + clroff);
                        c += 4;
                    }
                }
                else if(cdepth == 4)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = *((unsigned char *)&palette[c[0] >> 4] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[0] & 0xf] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[1] >> 4] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[1] & 0xf] + clroff);
                        c += 2;
                    }
                }
            }
            break;

        }
    }
}

void
VidConvert::rgb_to_gray(unsigned char *buf, gray **g)
{
    unsigned short *b = (unsigned short *)buf;
    unsigned char *c = (unsigned char *)buf;
    int do_mask = (and_mask != 0xff || or_mask != 0 || xor_mask != 0 || samp_off != 0);
    if((style & AQ_INDEX) && !palette)
        return;
    // note: this looks backwards because it is. the stuff down
    // below was mistakenly swapped, and this is "unswapping" it.
    //
    gray *conv555 = !swap_rb ? Colgray15_s : Colgray15;
    int *bluemul = !swap_rb ? m30 : m11;
    int *redmul = !swap_rb ? m11 : m30;
    for(int i = 0; i < crows; ++i)
    {
        gray *gr;
        if(upside_down)
            gr = &g[crows - i - 1][0];
        else
            gr = &g[i][0];
        if(style & AQ_RGB555)
        {
            for(int j = 0; j < ccols; j += 4)
            {
                gray g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
                g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
                g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
                g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
            }
        }
        else if(style & AQ_RGB24)
        {
            if(do_mask)
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    *gr++ = (redmul[MASKFUN(c[0])] + m59[MASKFUN(c[1])] + bluemul[MASKFUN(c[2])]) >> 7;
                    *gr++ = (redmul[MASKFUN(c[3])] + m59[MASKFUN(c[4])] + bluemul[MASKFUN(c[5])]) >> 7;
                    *gr++ = (redmul[MASKFUN(c[6])] + m59[MASKFUN(c[7])] + bluemul[MASKFUN(c[8])]) >> 7;
                    *gr++ = (redmul[MASKFUN(c[9])] + m59[MASKFUN(c[10])] + bluemul[MASKFUN(c[11])]) >> 7;
                    c += 12;
                }
            }
            else
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    *gr++ = (redmul[c[0]] + m59[c[1]] + bluemul[c[2]]) >> 7;
                    *gr++ = (redmul[c[3]] + m59[c[4]] + bluemul[c[5]]) >> 7;
                    *gr++ = (redmul[c[6]] + m59[c[7]] + bluemul[c[8]]) >> 7;
                    *gr++ = (redmul[c[9]] + m59[c[10]] + bluemul[c[11]]) >> 7;
                    c += 12;
                }
            }
        }
        else if(style & AQ_INDEX)
        {
            if(do_mask)
            {
                if(cdepth == 8)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[c[0]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[1]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[2]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[3]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        c += 4;

                    }
                }
                else if(cdepth == 4)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[(c[0] >> 4) & 0xf];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[(c[0] & 0xf)];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[(c[1] >> 4) & 0xf];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[1] & 0xf];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        c += 2;
                    }
                }
            }
            else
            {
                if(cdepth == 8)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[c[0]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[1]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[2]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[3]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        c += 4;
                    }
                }
                else if(cdepth == 4)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[(c[0] >> 4) & 0xf];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[(c[0] & 0xf)];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[(c[1] >> 4) & 0xf];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[1] & 0xf];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        c += 2;
                    }
                }
            }
        }
    }
}

#define PPM_ASSIGN_SWAP(a, r, g, b) PPM_ASSIGN((a), (b), (g), (r))
void
VidConvert::unpack_to_ppm(unsigned char *buf, pixel **g,
                          gray **yo, gray **cbo, gray **cro, int no_ycbcr)
{
    unsigned short *b = (unsigned short *)buf;
    unsigned char *c = (unsigned char *)buf;
    int do_mask = (and_mask != 0xff || or_mask != 0 || xor_mask != 0 || samp_off != 0);
    if((style & AQ_INDEX) && !palette)
        return;
    JSAMPIMAGE a = new JSAMPARRAY[3];
    a[0] = yo;
    a[1] = cbo;
    a[2] = cro;
    for(int i = 0; i < crows; ++i)
    {
        pixel *gr;
        pixel **grs;
        if(upside_down)
        {
            gr = &g[crows - i - 1][0];
            grs = &g[crows - i - 1];
        }
        else
        {
            gr = &g[i][0];
            grs = &g[i];
        }
        if(style & AQ_RGB555)
        {
            if(swap_rb)
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                }
            }
            else
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                }
            }
        }
        else if(style & AQ_RGB24)
        {
            if(swap_rb)
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN_SWAP(*gr, c[0], c[1], c[2]);
                    gr++;
                    PPM_ASSIGN_SWAP(*gr, c[3], c[4], c[5]);
                    gr++;
                    PPM_ASSIGN_SWAP(*gr, c[6], c[7], c[8]);
                    gr++;
                    PPM_ASSIGN_SWAP(*gr, c[9], c[10], c[11]);
                    gr++;
                    c += 12;
                }
            }
            else
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN(*gr, c[0], c[1], c[2]);
                    gr++;
                    PPM_ASSIGN(*gr, c[3], c[4], c[5]);
                    gr++;
                    PPM_ASSIGN(*gr, c[6], c[7], c[8]);
                    gr++;
                    PPM_ASSIGN(*gr, c[9], c[10], c[11]);
                    gr++;
                    c += 12;
                }
            }
        }
        else if(style & AQ_INDEX)
        {
            {
                if(cdepth == 8)
                {
                    if(swap_rb)
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[c[0]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[2]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[3]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 4;
                        }
                    }
                    else
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[c[0]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[2]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[3]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 4;
                        }
                    }
                }
                else if(cdepth == 4)
                {
                    if(swap_rb)
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[(c[0] >> 4) & 0xf];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[0] & 0xf)];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[1] >> 4) & 0xf];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1] & 0xf];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 2;
                        }
                    }
                    else
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[(c[0] >> 4) & 0xf];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[0] & 0xf)];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[1] >> 4) & 0xf];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1] & 0xf];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 2;
                        }
                    }
                }
            }
        }
        if(!no_ycbcr)
        {
            // perform YCrCb conversion right now
            // to take advantage of cache
            //
            rgb_ycc_convert((unsigned char **)grs, a,
                            upside_down ? (crows - i - 1) : i , ccols, 1);
        }
    }
    delete [] a;
}

gray **
VidConvert::get_chroma_plane(unsigned char *&c, int subsample)
{
    gray **p;
    int ch_cols = ccols / subsample;
    int ch_rows = crows / subsample;

    p = pgm_allocarray(ch_cols, ch_rows);

    if((autoconfig && !upside_down) || (!autoconfig && upside_down))
    {
        memcpy(&p[0][0], c, ch_cols * ch_rows);
        c += ch_cols * ch_rows;
    }
    else
    {
        for(int i = ch_rows - 1; i >= 0; --i)
        {
            gray *gr;
            gr = &p[i][0];
            memcpy(gr, c, ch_cols);
            c += ch_cols;
        }
    }
    return p;
}

// this is for nv21
// c should point to the first sample in the block of
// interleaved chroma samples
void
VidConvert::get_interleaved_chroma_planes(unsigned char *c, void*& vu_out, void*& vv_out, int subsample)
{
    int ch_cols = ccols / subsample;
    int ch_rows = crows / subsample;

    gray **u_out = pgm_allocarray(ch_cols, ch_rows);
    gray **v_out = pgm_allocarray(ch_cols, ch_rows);
    vu_out = (void *)u_out;
    vv_out = (void *)v_out;

    if((autoconfig && !upside_down) || (!autoconfig && upside_down))
    {
        gray *ut = &u_out[0][0];
        gray *vt = &v_out[0][0];
        gray *us = c;
        gray *vs = c + 1;
        for(int i = 0; i < ch_rows; ++i)
        {
            for(int j = 0; j < ch_cols; ++j)
            {
                *ut++ = *us;
                us += 2;
                *vt++ = *vs;
                vs += 2;
            }
        }
    }
    else
    {
        gray *us = c;
        gray *vs = c + 1;
        for(int i = ch_rows - 1; i >= 0; --i)
        {
            gray *ut = &u_out[i][0];
            gray *vt = &v_out[i][0];
            for(int j = 0; j < ch_cols; ++j)
            {
                *ut++ = *us;
                us += 2;
                *vt++ = *vs;
                vs += 2;
            }
        }
    }
}

void
VidConvert::unpack_to_planar(unsigned char *buf,
                             gray **yo, gray **cbo, gray **cro)
{
    // uyvy, subsampled chroma horizontally , but not vertically
    int flip = 0;
    if(upside_down)
    {
        flip = 1;
    }
    int yoff = 1;
    int cboff = 0;
    int croff = 2;
    if(style & AQ_YUY2)
    {
        yoff = 0;
        cboff = 1;
        croff = 3;
    }
    for(int i = 0; i < crows; ++i)
    {
        gray *yline;
        gray *cbline;
        gray *crline;
        if(flip)
        {
            yline = &yo[crows - i - 1][0];
            cbline = &cbo[crows / 2 - i / 2 - 1][0];
            crline = &cro[crows / 2 - i / 2 - 1][0];
        }
        else
        {
            yline = &yo[i][0];
            cbline = &cbo[i / 2][0];
            crline = &cro[i / 2][0];
        }
        int j;
        for(j = 0; j < ccols; ++j)
        {
            *yline++ = buf[j * 2 + yoff];
        }
        for(j = 0; j < ccols / 2; ++j)
        {
            if(i & 1)
            {
                *cbline = (*cbline + buf[j * 4 + cboff]) / 2;
                *crline = (*crline + buf[j * 4 + croff]) / 2;
                ++cbline;
                ++crline;
            }
            else
            {
                *cbline++ = buf[j * 4 + cboff];
                *crline++ = buf[j * 4 + croff];
            }
        }
        buf += 2 * ccols;
    }
}
