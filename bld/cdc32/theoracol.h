
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TheoraCOL_H
#define TheoraCOL_H
#include "colcod.h"
#include "coldec.h"
#include "theora/theoraenc.h"
#include "theora/theoradec.h"
#include "dwstr.h"


class CDCTheoraCoderColor : public DwCoderColor
{
public:
    CDCTheoraCoderColor();
    virtual ~CDCTheoraCoderColor();
    virtual DWBYTE *final_package(int &len);
    virtual DWBYTE * code_preprocess(gray **img_l, gray **img_cb, gray **img_cr,
                                     int cols, int rows, int is_ref, int& len_out, void *captured_ppm);
private:
    virtual void codeyuv(gray **img_l, gray **img_cb, gray **img_cr,
                         int cols, int rows, int is_ref, int& len_out, int subsamp);

    virtual void display_img_2b_coded(gray **g, int cols, int rows);
    virtual void display_img_2b_coded(pixel **g, int cols, int rows);
    virtual void display_img_2b_coded(gray **y, gray **cb, gray **cr, int cols, int rows);

private:
    int get_quality();
    th_info cur_info;
    th_enc_ctx *cur_coder_ctx;
    DwString working_buf;
    int cur_size;
    int cur_qual;
    int cur_rows;
    int cur_cols;

};

class CDCTheoraDecoderColor : public DwDecoderColor
{
public:
    CDCTheoraDecoderColor();
    virtual ~CDCTheoraDecoderColor();

    virtual void decode_from_stream(DWBYTE*& buf, int& len, void *&vimg, int &cols, int& rows);
    virtual void decode_postprocess(DWBYTE *buf, int len);
    virtual void display_decoded(void *p, int cols, int rows);
private:
    void setup_decoder_state(int fsize, int fqual);
    friend class MMChannel;
    int seeking_keyframe;
    th_dec_ctx *cur_dec_ctx;
    th_setup_info *setup_info;
    th_info cur_info;
    int cur_fqual;
    int cur_fsize;
    int cur_frows;
    int cur_fcols;

};
#endif
