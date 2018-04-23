
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tpgmdec.h 1.8 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef TPGMDEC_H
#define TPGMDEC_H
#include "tdecode.h"
#include "jtdecode.h"
#include "coldec.h"
#include "jcoldec.h"

class TPGMDecoder :
    public JPEGTDecoder
{
public:
    virtual ~TPGMDecoder();
    void convert(MAT m, void *app_img, int i, int j, QTAB *);
    virtual void *alloc_appimg(int blkw, int blkh);
    virtual void dealloc_refimg();
};

class TPGMDCTDecoder : public TPGMDecoder
{
public:
    virtual ~TPGMDCTDecoder() {}
    void inv_xform(MAT) {}
    void convert(MAT m, void *app_img, int i, int j, QTAB *);

};

#include "grayview.h"
class TPGMMSWDecoder : public TPGMDecoder
{
public:
    TPGMMSWDecoder() {
        gv = 0;
    }
    virtual ~TPGMMSWDecoder() {}
    grayview *gv;

    void convert(MAT m, void *app_img, int i, int j, QTAB *);
    void display_decoded(void *, int cols, int rows);
    int finish_line();
    int finish_block();
    void display_info(const char *);

    static void (*resize_video_window_callback)(int id, int cols, int rows);
    static void (*display_info_in_titlebar_callback)(int id, const char *str);
};

class TPGMMSWDecoderColor : public JPEGDecoderColor
{
public:
    virtual ~TPGMMSWDecoderColor() {}
    void display_decoded(void *, int cols, int rows);
    void display_info(const char *);
    static void (*resize_video_window_callback)(int id, int cols, int rows);
    static void (*display_info_in_titlebar_callback)(int id, const char *str);
};

class TPGMMSWDCTDecoder : public TPGMMSWDecoder
{
public:
    virtual ~TPGMMSWDCTDecoder() {}
    void convert(MAT m, void *app_img, int i, int j, QTAB *);
    void inv_xform(MAT) {}
};

#endif

