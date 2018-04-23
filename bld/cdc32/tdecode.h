
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tdecode.h 1.16 1999/01/10 16:10:53 dwight Checkpoint $
 */
#ifndef TDECODE_H
#define TDECODE_H
#include "matcom.h"
#include "pbmcfg.h"
#include "codec.h"
#include "packbits.h"
#include "nyb.h"
#include "rlc.h"
#include "qtab.h"

class TDecoder : public DwDecoder
{
protected:

    XFRM ref_xform;

    int fis_ref;
    int entropy_code;
    int still;
    int has_error;
    Unpackbits unpacker;
    Unpackbits ctrl_unpacker;
    Unpackbits fvlc_unpacker;
    DPCMStreamIn dpcm_max_mag;
    BinaryRLD use_error_decoder;
    BinaryRLD hold_decoder;

    virtual unsigned short get_chroma_info(DWBYTE *frm, unsigned short *p, int& has_chroma) {
        return 0;
    }

public:

    void destr_fun();
    void *ref_img;
    int ri_cols;
    int ri_rows;

    TDecoder();
    virtual ~TDecoder();

    void error();
    void reset();
    int get_error() {
        return has_error;
    }
    void decode_from_stream(DWBYTE*& strm, int& len, void *&, int& cols, int& rows);
    virtual void decode_frame(DWBYTE* control,
                              DWBYTE* over, BITBUFT* img, BITBUFT *vlc,
                              BITBUFT *hold_buf, BITBUFT *use_error_buf,
                              void *&app_img, int cols, int rows, int is_ref);

    virtual void inv_xform(MAT m);

    virtual void convert(MAT m, void *app_img, int i, int j, QTAB *) = 0;
    virtual void *alloc_appimg(int blkw, int blkh) = 0;
    virtual void dealloc_refimg() = 0;

};

class TDecoderNP : virtual public TDecoder
{
public:
    virtual void decode_frame(DWBYTE *control, DWBYTE *over, BITBUFT *img,
                              BITBUFT *vlc, BITBUFT* hold_buf, BITBUFT* use_error_buf,
                              void *& app_img,
                              int cols, int rows, int is_ref);
};
#endif
