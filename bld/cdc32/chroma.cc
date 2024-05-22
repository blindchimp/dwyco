#ifdef DWYCO_DCT_CODER
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/chroma.cc 1.10 1997/11/25 20:41:03 dwight Stable095 $
 */
//
// this chroma codec is derived from
// the luma codec so it can use most of
// frame manipulation functions which
// are the same for each. when the
// chroma codec is created, you have to
// give it a luma codec so it can use
// the decisions made there to properly
// encode the chroma planes. one chroma
// plane is encoded. you need two of these
// objects to encode both the u and v planes.
//
#include "jtcode.h"
#include "pbmcfg.h"
#include "matcom.h"
#include "qpol.h"
#include "packbits.h"
#include "jdunpack.h"
#define JPEG_INTERNALS
#include "jmorecfg.h"
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"
#include "jchuff.h"
#include "jdhuff.h"
#include "macs.h"
#include "mo.h"
#include "chroma.h"

#define VERB(x)

extern JSAMPLE *Range_limit;

JPEGTCoderChroma::JPEGTCoderChroma(JPEGTCoder *l)
{
    luma = l;
}

JPEGTCoderChroma::~JPEGTCoderChroma()
{
}


void
JPEGTCoderChroma::code_frame(gray **frm, int cols, int rows, int is_ref, const char *polname)
{
    int i, j;
    int ib, jb;
    int low_variance;
    MAT tpxs;
    ELTYPE *tpxsp[MSZ];
    for(i = 0; i < MSZ; ++i)
        tpxsp[i] = &tpxs[i][0];
    ELTYPE **tmp_pixels = &tpxsp[0];

    if((cols & ~STRUNC) != fcols || (rows & ~STRUNC) != frows || sampling != fsampling)
    {
        // changed dimensions on us, have to reset
        // everthing.
        is_ref = 1;
        if(decoded_frame && frows)
            pgm_freearray(decoded_frame, frows);
        if(decoded_frame_now && frows)
            pgm_freearray(decoded_frame_now, frows);
        if(last_frm && frows)
            pgm_freearray(last_frm, frows);
        decoded_frame_now = 0;
        decoded_frame = 0;
        last_frm = 0;
    }

    rows &= ~STRUNC;
    cols &= ~STRUNC;
    frame_buf = coded_frame;
    outpacker.init();
#ifdef RECORD
    extern Packbits *CTracking;
    CTracking = &outpacker;
#endif
    if(last_frm == 0 || still)
        is_ref = 1;
    if(decoded_frame == 0)
        decoded_frame = pgm_allocarray(cols, rows);
    if(decoded_frame_now == 0)
        decoded_frame_now = pgm_allocarray(cols, rows);
    fis_ref = is_ref;
    fcols = cols;
    frows = rows;
    fsampling = sampling;
    QPolicy *qpp;
    if(!Policy.find(polname, qpp))
        return;
    start_frame();
    int last_dc_val = 0;
    int *which_dc_val;
    int dc_val;
    for(i = 0, ib = 0; i < rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < cols; j += MSZ, ++jb)
        {
            start_block();
            VERB(
                printf("c(%d, %d) ", ib, jb);
            )

            int chroma_motion = 0;
            if(!is_ref)
            {
                // check the four blocks in the luma
                // plane to see if they are changing
                // if not, then don't encode this block.
                if(fsampling == SAMP411)
                {
                    // check 16 luma blocks
                    for(int il = 0; il < 4; ++il)
                        for(int jl = 0; jl < 4; ++jl)
                            if(luma->m_motion[4 * ib + il][4 * jb + jl])
                            {
                                chroma_motion = 1;
                                goto out;
                            }
out:
                    ;
                }
                else if(fsampling == SAMP422)
                {
                    if(luma->m_motion[2 * ib][2 * jb] ||
                            luma->m_motion[2 * ib + 1][2 * jb] ||
                            luma->m_motion[2 * ib][2 * jb + 1] ||
                            luma->m_motion[2 * ib + 1][2 * jb + 1])
                    {
                        chroma_motion = 1;
                    }
                }
#if 0
                if(luma->m_motion[2 * ib][2 * jb] ||
                        luma->m_motion[2 * ib + 1][2 * jb] ||
                        luma->m_motion[2 * ib][2 * jb + 1] ||
                        luma->m_motion[2 * ib + 1][2 * jb + 1])
                {
                    chroma_motion = 1;
                }
#endif
                VERB(
                    printf("cmotion = %d ", chroma_motion);
                )
            }
            else
            {
                chroma_motion = 0;
            }
            DwString blev;
            blev = qpp->names[REFERENCE + Q_CHROMA_OFFSET];
            QTAB *qp;
            if(!QTables.find(blev, qp))
                continue;
#if 0
            if(ib == 0 && jb == 0)
                outpacker.addbits_raw(qp->id, 5, frame_buf);
#endif
            if(!chroma_motion && !is_ref)

            {
                VERB(
                    printf("c(%d, %d) hold\n", ib, jb);
                )
#ifdef GENHIST
                Ts->add_sample(SYM_HOLD, 1);
#endif
                goto holdit;
            }
            if(!is_ref)
            {
#ifdef GENHIST
                Ts->add_sample(SYM_HOLD, 0);
#endif
            }
            copy_block(frm, i, j);

            VERB(
                printf("cvar %f ", bvar);
            )

            low_variance = 0;
            //
            // check the four luma blocks
            // for this chroma block. if the
            // variance for one of them is
            // high, then chroma block is high.
            //
            if(fsampling == SAMP411 && !is_ref)
            {
                for(int il = 0; il < 4; ++il)
                    for(int jl = 0; jl < 4; ++jl)
                        if(luma->m_hv[4 * ib + il][4 * jb + jl])
                        {
                            low_variance = 0;
                            goto out2;
                        }
                low_variance = 1;
out2:
                ;
            }
            else if(fsampling == SAMP422 && !is_ref)
            {
                if(!(luma->m_hv[2 * ib][2 * jb] ||
                        luma->m_hv[2 * ib + 1][2 * jb] ||
                        luma->m_hv[2 * ib][2 * jb + 1] ||
                        luma->m_hv[2 * ib + 1][2 * jb + 1]))
                {
                    low_variance = 1;
                }
                else
                    low_variance = 0;
            }
#if 0
            if(!(luma->m_hv[2 * ib][2 * jb] ||
                    luma->m_hv[2 * ib + 1][2 * jb] ||
                    luma->m_hv[2 * ib][2 * jb + 1] ||
                    luma->m_hv[2 * ib + 1][2 * jb + 1]) && !is_ref)
#endif

                if(low_variance && !is_ref)
                {
                    xform(tmp, qp);
                    inv_xform(tmp, i, j, qp);
                    which_dc_val = &last_dc_val;
                    // guarantees we may have to take a couple of
                    // frames to notice error settling down again.
                    VERB(
                        printf("clow variance ");
                    )
                    low_variance = 1;
                    goto quantize;
                }
#if 0
                else if(luma->use_motion_comp && !is_ref &&
                        i >= 8 && i < rows - 8 && j >= 8 && j < cols - 8)
                {
                    // try to see if motion comp may help
                    // for chroma, we don't do any motion comp
                    // for now.
                    int vi, vj;
                    MAT diff_tmp;
                    bm_motion_estimate(last_frm, &tmp[0][0], i, j, vi, vj);
                    //lbm_motion_estimate8x8(last_frm, &tmp[0][0], i, j, vi, vj);
                    double v = mo_diff_block(&diff_tmp[0][0], i - MV_ZERO + vi, j - MV_ZERO + vj, qp);
                    ++Total_mo;
                    if(v < bvar && (vi != MV_ZERO || vj != MV_ZERO))
                    {
                        ++Succ_mo;
                        VERB(
                            printf("cvdiff (%f - %f) %f\n", bvar, v, bvar - v);
                        )
#ifdef GENHIST
                        Ts->add_sample(SYM_VEC, vi);
                        Ts->add_sample(SYM_VEC, vj);
#endif
                        mocomp = 1;
                        output_mv(vi, vj);
                        xform(diff_tmp, qp);
                        bcopy(&diff_tmp[0][0], &tmp[0][0], sizeof(MAT));
                        jpeg_idct_islow((JCOEFPTR)&diff_tmp[0][0], tmp_pixels, 0, qp, 0);
                        motion_compensate(tmp_pixels, i, j, vi, vj);
                        //which_dc_val = &mocomp_dc_val;
                        which_dc_val = &last_dc_val;
                        goto quantize;
                    }
                    else
                    {
                        double v = mo_diff_block(&diff_tmp[0][0], i, j, qp);
                        xform(diff_tmp, qp);
                        bcopy(&diff_tmp[0][0], &tmp[0][0], sizeof(MAT));
                        jpeg_idct_islow((JCOEFPTR)&diff_tmp[0][0], tmp_pixels, 0, qp, 0);
                        motion_compensate(tmp_pixels, i, j, MV_ZERO, MV_ZERO);
                        //which_dc_val = &mocomp_dc_val;
                        which_dc_val = &last_dc_val;
                        use_error = 1;
                        goto quantize;
                    }
                }
                else if(!is_ref && mean_motion >= mean_motion_thresh)
#endif
                    else if(!is_ref) /*&& chroma_motion)*/
                    {
                        // use zero vector
                        MAT diff_tmp;
                        mo_diff_block(&diff_tmp[0][0], i, j, qp);
                        xform(diff_tmp, qp);
                        bcopy(&diff_tmp[0][0], &tmp[0][0], sizeof(MAT));
                        jpeg_idct_islow((JCOEFPTR)&diff_tmp[0][0], tmp_pixels, 0, qp, 0);
                        motion_compensate(tmp_pixels, i, j, MV_ZERO, MV_ZERO);
                        //which_dc_val = &mocomp_dc_val;
                        which_dc_val = &last_dc_val;
                        VERB(
                            printf("chroma err %d %d\n", ib, jb);
                        )
                        goto quantize;
                    }
            xform(tmp, qp);
            inv_xform(tmp, i, j, qp);
            which_dc_val = &last_dc_val;

quantize:
            if(!is_ref)
            {
#ifdef GENHIST
                Ts->add_sample(SYM_USE_ERROR, use_error);
#endif
                VERB(
                    printf("c use err: %d\n", use_error);
                )
            }
            dc_val = *which_dc_val;
            encode_one_block ((JCOEF *)tmp, dc_val,
                              Dc_derived_tbls_c[1], Ac_derived_tbls_c[1], outpacker, frame_buf, cpu_save, coeff_tweak);
            *which_dc_val = dc_val;
            goto done_block;

holdit:
            {

                // block is on hold, must copy it from previous
                // frame
                int r;
                for(r = i; r < i + 8; ++r)
                {
                    *(dwINT32 *)&((gray **)decoded_frame_now)[r][j] = *(dwINT32 *)&decoded_frame[r][j] ;
                    *((dwINT32 *)&((gray **)decoded_frame_now)[r][j] + 1) = *((dwINT32 *)&decoded_frame[r][j] + 1);
                }
            }
done_block:
            finish_block();
        }
        finish_line();
    }
    outpacker.flush(frame_buf);
    if(last_frm != 0)
        pgm_freearray(last_frm, frows);
    last_frm = frm;
    last_was_ref = is_ref;
    XFRM *xf_tmp = ref_xform_now;
    ref_xform_now = ref_xform_last;
    ref_xform_last = xf_tmp;
    gray **df = decoded_frame;
    decoded_frame = decoded_frame_now;
    decoded_frame_now = df;
    finish_frame();
}
#endif
