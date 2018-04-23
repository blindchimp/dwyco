
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/colcod.h 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef COLCOD_H
#define COLCOD_H

#include "pbmcfg.h"
#include "matcom.h"
#include "codec.h"

void oopanic(const char *);

// this class encapsulates 3 coders:
// one for luma and 2 for chroma.
// main task is coordinating all the
// function calls to each codec so
// luma is twice the res of chroma
// codecs, and packaging up the
// resulting bit streams.
//

class DwCoderColor : public DwCoder
{
public:
    DwCoderColor();
    virtual ~DwCoderColor();


    DWBYTE *code_preprocess(gray **img, int cols, int rows, int is_ref, int& len_out) {
        return 0;
    }
    DWBYTE *code_preprocess(gray **l_img, gray **cb_img, gray **cr_img,
                            int cols, int rows, int is_ref, int& len_out, void *captured_ppm = 0);
    void codeyuv(gray **l_img, gray **cb_img, gray **cr_img, int cols, int rows, int is_ref, int& len_out, int subsamp);
    void set_crop(int);
    void set_subsample(int);
    void set_crop_window(int x, int y, int w, int h);
    void set_policy(const char *pol);
    void set_pad(int);
    void set_sampling(int);
    virtual void set_special_parms(double, double, double, int, int, int, int, int, unsigned int);

    // override in derived classes to see image just
    // after preprocessing and before it is sent to coder
    virtual void display_img_2b_coded(gray **g, int cols, int rows) {}
    virtual void display_img_2b_coded(pixel **g, int cols, int rows) {}
    virtual void display_img_2b_coded(gray **y, gray **cb, gray **cr, int cols, int rows) {}

    virtual void code_frame(gray **, int cols, int rows, int is_ref, const char *policy = "default") {
        oopanic("chroma code frame?");
    }
    virtual DWBYTE *final_package(int& len) = 0;

public:
    DwCoder *luma;
    DwCoder *cr;
    DwCoder *cb;
};

#endif
