
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqvfw.h 1.25 1999/01/10 16:10:40 dwight Checkpoint $
 */
#ifndef VIDCVT_H
#define VIDCVT_H
#ifdef __BORLANDC__
#include <mem.h>
#else
#include <string.h>
#endif
#include <stddef.h>
#include "pbmcfg.h"

// this class tries to encapsulate the process of
// converting a raw input format into a planar format
// that can be used by our video codecs.
// it takes as input a buffer of bytes, a length, and the
// dimensions (in pixels) of the grabbed frame. the input can be
// an RGB format or a YUV (either planar or interleaved) format.
// there is limited auto-detection based on possible sizes for the
// input, in case the input format cannot be determined in advance.
//
// the output is either:
// * a planar YUV format, with the luma dimensions the same as the input dimensions,
//  and the chroma dimensions subsampled as in YUV12 or YUV9 (depending on
//  how you set various parameters.
//
// * an RGB format PPM, which is unpacked from whatever RGB format
//  is input, *and* optionally a YUV representation of the same
//  frame that will be suitable for the codecs.
//
// the reason for the RGB+YUV output is that in a lot of cases, it
// is useful to have both representations, one for coding, and one
// for display on a screen.
//
// this class can also perform various plane-swap operations, and
// grayscale conversion, and upside-down transformations. most of
// this is here because historically, video capture hardware has
// never had a standard way of representing the captured video
// data, and it is somewhat ad-hoc getting the right flip/rb/uv
// etc.
//
// WARNING: autoconfig mode hasn't been tested in a LONG time...
// i got tired of fighting it, and just decided to hardwire the
// input format to packed RGB in most cases, so this class ends up just
// being used to convert that to PPM and YUV12
//

class VidConvert
{
public:
    VidConvert();
    ~VidConvert();

    int ccols;
    int crows;
    int cdepth;
    int autoconfig;
    int swap_rb;
    int swap_uv;
    unsigned long compression; // (DWORD) normally really the format
    unsigned char *palinfo;

    void set_format(int);
    void set_use_plane(int i) {
        style &= ~(AQ_ALL_PLANES|AQ_ONE_PLANE);
        style |= i;
    }
    void set_which_plane(int i) {
        style &= ~AQ_PLANES;
        style |= i;
    }
    void set_style(int);
    void add_style(int);
    void remove_style(int);
    void set_upside_down(int);
    unsigned int xor_mask;
    unsigned int and_mask;
    unsigned int or_mask;
    int samp_off;
    int force_gray;
    void *convert_data(unsigned char *buf, int buflen, int& cols, int& rows,
                       void*& y, void*& cb, void*& cr, int& fmt, int no_ycbcr );

private:
    typedef struct RGBQUAD {
        unsigned char    rgbBlue;
        unsigned char    rgbGreen;
        unsigned char    rgbRed;
        unsigned char    rgbReserved;
    } RGBQUAD;

    int style;
    int upside_down;
    RGBQUAD *palette;
    int is_palette;

    void rgb_to_gray(unsigned char *buf, gray **g);
    void grab_one_plane(unsigned char *buf, gray **g);
    gray **get_chroma_plane(unsigned char *&, int subsamp);
    void unpack_to_ppm(unsigned char *in, pixel **out, gray **y, gray **cb, gray **cr, int no_ycbcr);
    void unpack_to_planar(unsigned char *in, gray **y, gray **cb, gray **cr);
    void get_interleaved_chroma_planes(unsigned char *c, void*& u_out, void*& v_out, int subsample);
};

#endif
