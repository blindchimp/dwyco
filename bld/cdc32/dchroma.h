
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dchroma.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef DCHROMA_H
#define DCHROMA_H
#include "matcom.h"
#include "tpgmdec.h"

class JPEGTDecoderChroma : public TPGMDCTDecoder
{
public:
    JPEGTDecoderChroma(int plane = 0, JPEGTDecoder *l = 0);
    virtual ~JPEGTDecoderChroma();


    //void display_decoded(gray **, int, int);
    unsigned short get_chroma_info(DWBYTE *frm, unsigned short *p, int& has_chroma);
    virtual void decode_frame(DWBYTE* ctrl, DWBYTE* overhead, BITBUFT* img,
                              BITBUFT *vlc, BITBUFT *hold_buf, BITBUFT *use_error_buf,
                              void *&app_img, int icols, int irows, int is_ref);

private:
    int which_plane;
    JPEGTDecoder *luma;
};

#endif
