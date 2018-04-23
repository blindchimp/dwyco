
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jtcode.h 1.10 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef JTCODE_H
#define JTCODE_H
#include "matcom.h"
#include "qtab.h"
#include "pbmcfg.h"
#include "nyb.h"


class JPEGTCoder : public TCoder
{
    friend class JPEGTCoderChroma;
    friend class JPEGTCoder3Chroma;
public:
    JPEGTCoder();
    virtual ~JPEGTCoder();
    void xform(MAT m, QTAB *q);
    void inv_xform(MAT m, int i, int j, QTAB *qp);
    double mo_diff_block(int *, int si, int sj, QTAB *);
    void diff_block(gray **img, gray **last_img, int si, int sj);
    void code_frame(gray **frm, int cols, int rows, int is_ref, const char *polname = "default");

    int cpu_save;
    int use_motion_comp;

    virtual double predict(int ib, int jb);
    virtual void predict_recon(int is_ref, int ib, int jb, QTAB&);
    void set_special_parms(double mae_hold, double mae_switch, double mad, int temp_comp,
                           int noise_clip, int ent_code, int cpusave, int motion_comp, unsigned int coeff_tweaker);

protected:
    XFRM *ref_xform_last;
    XFRM *ref_xform_now;
    gray **decoded_frame;
    gray **decoded_frame_now;
    int decoded[BLKR][BLKC];
    int comped[BLKR][BLKC];
    int m_hv[BLKR][BLKC];
    int m_motion[BLKR][BLKC];
    double mae_tmp();
    BinaryRLC mocomp_encoder;
    void output_mv(int vi, int vj);
    void motion_compensate(ELTYPE **tmp_pixels, int i, int j, int vi, int vj);
    DPCMStream mvi;
    DPCMStream mvj;
    unsigned int coeff_tweak;

};
#endif
