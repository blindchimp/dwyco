
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jtdecode.h 1.12 1999/01/10 16:10:49 dwight Checkpoint $
 */
#ifndef JTDECODE_H
#define JTDECODE_H
#include "tdecode.h"
#include "jdunpack.h"
#include "nyb.h"

class JPEGTDecoder : public TDecoder
{
    friend class JPEGTDecoderChroma;
    friend class TPGMMSWDecoderColor;
    friend class JPEGTDecoder3Chroma;
public:
    JPEGTDecoder();
    virtual ~JPEGTDecoder();
    virtual void decode_frame(DWBYTE* ctrl, DWBYTE* overhead, BITBUFT* img,
                              BITBUFT *vlc, BITBUFT *hold_buf, BITBUFT *use_error_buf,
                              void *&app_img, int icols, int irows, int is_ref);
protected:
    gray **last_frame;
    JDUnpackbits junpacker;
    BinaryRLD mocomp_decoder;
    DPCMStream mvi;
    DPCMStream mvj;
    int b_ue[BLKR][BLKC];
    int b_hold[BLKR][BLKC];
    int qid;
    void get_mv(int& vi, int& vj, BITBUFT*& vlc);
    int decode_vec(BITBUFT*& vlc);

};
#endif
