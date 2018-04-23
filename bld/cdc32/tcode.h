
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tcode.h 1.15 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef TCODE_H
#define TCODE_H
#include "pbmcfg.h"
#include "matcom.h"
#include "codec.h"
#include "packbits.h"
#include "nyb.h"
#include "rlc.h"
class QTAB;
#define HOLD_BLOCK 0xaa
#define NO_MAG 16
#define NQUADS 1
#define QUADSZ (MSZ / NQUADS)
#define TC_DEFLT_ROWS 243
#define TC_DEFLT_COLS 360
#define CHROMA_OFFSET 16

class TCoder : public DwCoder
{
protected:

    XFRM ref_xform;
    //int cols, rows;
    gray **last_frm;
    BLKFEA d_mse;
    int hold[BLKR][BLKC];
    double bvar;
    int fis_ref;
    int fcols;
    int frows;
    int last_was_ref;
    Packbits outpacker;
    Packbits ctrl_packer;
    Packbits fvlc_packer;
    DPCMStreamOut dpcm_max_mag;
    BinaryRLC hold_encoder;
    BinaryRLC use_error_encoder;
    MAT tmp;
    MAT pred;
    MAT xform_res;

protected:
    virtual double predict(int ib, int jb);
    virtual void predict_recon(int is_ref, int ib, int jb, QTAB&);
    double mae_tmp();

public:
    double mse_hold_thresh;
    double mse_refine_thresh;
    double stdev_motion_thresh;
    double mean_motion_thresh;
    double variance_thresh;
    int noise_clip;
    int motion_only;
    int entropy_code;
    int still;

    BITBUFT *coded_frame;
    BITBUFT *frame_buf;
    BITBUFT *frame_vlc_coeff;
    BITBUFT *frame_vlc_buf;
    BITBUFT *ue_buf;
    BITBUFT *h_buf;

    // quantizer info
    BITBUFT *control_info;
    BITBUFT *ctrl_buf;
    // magnitudes
    BITBUFT *overhead;
    BITBUFT *over_buf;
    BITBUFT *use_error_buf;
    BITBUFT *hold_buf;

    //TCoder(int cols = TC_DEFLT_COLS, int rows = TC_DEFLT_ROWS);
    TCoder();
    virtual ~TCoder();

    virtual void xform(MAT m, QTAB *q);
    virtual void inv_xform(MAT m, int i, int j, QTAB *qp);
    virtual void copy_block(gray **img, int si, int sj);
    virtual void diff_block(gray **img, gray **last_img, int si, int sj) {}

    void reset();

    void set_special_parms(double mae_hold, double mae_switch, double mad, int temp_comp,
                           int noise_clip, int ent_code, int, int, unsigned int);
    void code_frame(gray **frm, int cols, int rows, int is_ref, const char *polname = "default");
    DWBYTE *final_package(int& len_out);
};

class TCoderNP : virtual public TCoder
{
protected:
    virtual double predict(int ib, int jb) {
        return 100.;
    }
    virtual void predict_recon(int is_ref, int ib, int jb, QTAB&) {}
};
double compute_motion(gray **f1, gray **f0, int si, int sj, int clip,
                      double& mean_out);
#endif
