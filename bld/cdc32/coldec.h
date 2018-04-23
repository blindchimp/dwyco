
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/coldec.h 1.8 1999/01/10 16:10:45 dwight Checkpoint $
 */
#ifndef COLDEC_H
#define COLDEC_H
#include "matcom.h"
#include "codec.h"
class QTAB;

//class DwDecoderColor : public TDecoder
class DwDecoderColor : public DwDecoder
{
private:
    // klugey, but it does keep people from using this class
    // directly or deriving it from it accidently.
    friend class JPEGDecoderColor;
    friend class CDCSpazDecoderColor;
    friend class JPEGTDecoder3Color;
    friend class CDCTheoraDecoderColor;
    DwDecoderColor();
public:
    virtual ~DwDecoderColor();

    void decode_postprocess(DWBYTE *buf, int len);
    void decode_from_stream(DWBYTE*& buf, int& len, void *&vimg, int &cols, int& rows);
    void convert(MAT m, void *app_img, int i, int j, QTAB *) {}
    virtual void *alloc_appimg(int blkw, int blkh) {
        oopanic("bad alloc_appimg");
        return 0;
    }
    virtual void dealloc_refimg() {
        oopanic("bad dealloc refimg");
    }

protected:

    DwDecoder *luma;
    DwDecoder *cb;
    DwDecoder *cr;

};

#endif
