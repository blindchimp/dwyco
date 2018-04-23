
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/codec.h 1.10 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef CODEC_H
#define CODEC_H
#include "pbmcfg.h"
#include "matcom.h"
#include "dwvec.h"

class DwCodec
{
public:
    DwCodec() {
        is_busy = 0;
        gv_id = 0;
    }
    virtual ~DwCodec() {}
    virtual int busy();
    virtual void set_busy(int b);
    virtual int start_line() {
        return 1;
    }
    virtual int finish_line() {
        return 1;
    }
    virtual int start_block() {
        return 1;
    }
    virtual int finish_block() {
        return 1;
    }
    virtual int start_frame() {
        return 1;
    }
    virtual int finish_frame() {
        return 1;
    }
    virtual void display_info(const char *str) {}

    int gv_id;

    // in case we have to put the video in a
    // bunch of different places
    DwVec<int> gvs;

private:
    int is_busy;

};

class DwDecoder;
class DwCoder : public DwCodec
{
    friend class DwCoderColor;
public:

    DwCoder();
    virtual ~DwCoder();

    // note: images given to this function are
    // owned by this function (they may be stored, or deleted.)
    // if you need to keep the image, copy it before
    // giving to this function.
    virtual DWBYTE *code_preprocess(gray **img, int cols, int rows, int is_ref, int& len_out);
    virtual DWBYTE *code_preprocess(gray **y, gray **cb, gray **cr,
                                    int cols, int rows, int is_ref, int& len_out, void *captured_ppm = 0) {
        return 0;
    }
    virtual void set_crop(int);
    virtual void set_subsample(int);
    virtual void set_crop_window(int x, int y, int w, int h);
    virtual void set_policy(const char *pol);
    virtual void set_pad(int);
    virtual void set_sampling(int);
    virtual int get_sampling();
    virtual void set_special_parms(double, double, double, int, int, int, int, int, unsigned int) {}

    // override in derived classes to see image just
    // after preprocessing and before it is sent to coder
    virtual void display_img_2b_coded(gray **g, int cols, int rows) {}
    virtual void display_img_2b_coded(gray **y, gray **cb, gray **cr, int cols, int rows) {}

    virtual void code_frame(gray **, int cols, int rows, int is_ref, const char *policy = "default") = 0;
    virtual DWBYTE *final_package(int& len) = 0;

    int inhibit_coding; // set this to 1 to only show preprocessed image

protected:
    // cropping info
    int do_crop; // 1 = crop before padding or clipping
    int x;
    int y;
    int w;
    int h;
    // other preprocessing option
    int do_subsample;
    int do_pad; // 1 = pad image to blocksize, 0 = clip
    // coding policy option
    char *policy;
    int sampling;
};

#define SAMP444 0
#define SAMP422 1
#define SAMP411 2
#define SAMP400 4
#define SAMPYUV9 5
#define SAMPYUV12 6

class DwDecoder : public DwCodec
{
public:
    DwDecoder();

    // note: this function also displays the decoded image
    virtual void decode_postprocess(DWBYTE *buf, int len);

    // override in derived to display decoded after postprocessing
    virtual void display_decoded(void *, int cols, int rows) {}

    virtual void decode_from_stream(DWBYTE*& buf, int& len, void *&vimg, int &cols, int& rows) = 0;

    virtual void set_upsample(int);
    virtual void set_tweak(unsigned int);
    virtual void set_sampling(int);
    virtual int get_sampling();
    int decode_color;
    int force_gray;

private:
    int do_upsample;
    int sampling;
protected:
    unsigned int tweak;


};



#endif
