
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jtcode.cc 1.24 1999/01/10 16:09:34 dwight Checkpoint $
 */
#include "jtcode.h"
#include "pbmcfg.h"
#include "matcom.h"
#include "qpol.h"
#include "packbits.h"
#include "jdunpack.h"
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"
#include "jchuff.h"
#include "jdhuff.h"
#include "macs.h"
#include "pred.h"
#include "mo.h"
#include "sqrs.h"
#include "statfun.h"

#define VERB(x)
//extern int Verbose;
[[noreturn]] void oopanic(const char *);

int MV_codes[16] =
{
    0x17, 0x7, 0x9, 0xb, 0x7, 0x3, 0x3, 0x3, 0x1, 0x2, 0x2, 0x2, 0x6, 0xa, 0x8, 0x6
};

int MV_code_lens[16] =
{
    10, 8, 8, 8, 7, 5, 4, 3, 1, 3, 4, 5, 7, 8, 8, 8
};

void
JPEGTCoder::output_mv(int vi, int vj)
{
    int out = mvi.dpcm(vi);
    if(out < -8)
        out += 16;
    else if(out > 7)
        out -= 16;
    out += 8;
    fvlc_packer.addbits_raw(MV_codes[out], MV_code_lens[out], frame_vlc_buf);

    out = mvj.dpcm(vj);
    if(out < -8)
        out += 16;
    else if(out > 7)
        out -= 16;
    out += 8;
    fvlc_packer.addbits_raw(MV_codes[out], MV_code_lens[out], frame_vlc_buf);
}

extern JSAMPLE *Range_limit;

JPEGTCoder::JPEGTCoder()
#if 0
    :	h1(1024, 255), h2(1024, 255), h3(1024, 255), h4(1024, 255)
#endif
{
    cpu_save = 64;
    use_motion_comp = 0;
    decoded_frame = 0;
    decoded_frame_now = 0;
    ref_xform_now = &ref_xform;
#if 0
    ref_xform_last = &ref_xform2;
#endif
    coeff_tweak = 0;
}

JPEGTCoder::~JPEGTCoder()
{
    if(decoded_frame)
        pgm_freearray(decoded_frame, frows);
    if(decoded_frame_now)
        pgm_freearray(decoded_frame_now, frows);
#if 0
    int n = h1.num_bins();
    FILE *f1 = fopen("h1.prn", "w");
    FILE *f2 = fopen("h2.prn", "w");
    FILE *f3 = fopen("h3.prn", "w");
    FILE *f4 = fopen("h4.prn", "w");
    for(int i = 0; i < n; ++i)
    {
        fprintf(f1, "%d\n", h1.get_bin(i));
        fprintf(f2, "%d\n", h2.get_bin(i));
        fprintf(f3, "%d\n", h3.get_bin(i));
        fprintf(f4, "%d\n", h4.get_bin(i));
    }
    fclose(f1);
    fclose(f2);
    fclose(f3);
    fclose(f4);
#endif
}

void
JPEGTCoder::set_special_parms(double mae_hold, double mae_switch, double mad,
                              int temp_comp, int noise, int ent_code, int cpu_save_inp, int motion_comp, unsigned int coeff_tweak_inp)
{
    mse_hold_thresh = mae_hold;
    mse_refine_thresh = mae_switch;
    mean_motion_thresh = mad;
    motion_only = temp_comp;
    noise_clip = noise;
    entropy_code = ent_code;
    cpu_save = cpu_save_inp;
    use_motion_comp = motion_comp;
    coeff_tweak = coeff_tweak_inp;
}

void
JPEGTCoder::inv_xform(MAT m, int i, int j, QTAB *qp)
{
    jpeg_idct_islow((JCOEFPTR)&m[0][0], &decoded_frame_now[i], j, qp, 1);
}
double
JPEGTCoder::predict(int ib, int jb)
{
    bcopy(&tmp[0][0], &xform_res[0][0], sizeof(MAT));
    ELTYPE *mp = &(*ref_xform_last)[ib][jb][0][0];
    int idx;
    for(int i = 0; i < cpu_save; ++i)
    {
        int tp;
        int idx;
        idx = jpeg_natural_order[i];
        tp = mp[idx];
        ((ELTYPE *)tmp)[idx] -= tp;
        ((ELTYPE *)pred)[idx] = tp;

    }

#if 0
// bc4.5 has a compiler bug that makes this not work...
#define loop_body(x) \
	idx = jpeg_natural_order[i + (x)]; \
	tp = PREDICT(mp[idx]); \
	((ELTYPE *)pred)[idx] = tp; \
	foo = ((ELTYPE *)tmp)[idx]; \
	foo -= tp; \
	((ELTYPE *)tmp)[idx] = foo;

#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

    for(int i = 0; i < cpu_save; i += 4)
    {
        ELTYPE tp;
        int foo;
        loop_body4(0)
        loop_body4(4)
#if MSZ == 16
        loop_body4(8)
        loop_body4(12)
#endif
    }
#undef loop_body
#undef loop_body4
#endif
    double d = mae_tmp();
    return d;
}

void
JPEGTCoder::predict_recon(int is_ref, int ib, int jb, QTAB& q)
{
    if(!is_ref)
    {
        if(q.reconfun != 0)
        {
            // get untouched coeffs
            oopanic("reconfun != 0");
            bcopy(&pred[0][0], &ref_xform[ib][jb][0][0], sizeof(MAT));
            QTAB_RECONFUNP f = q.reconfun;
            (this->*f)(ib, jb);
            return;
        }
        //ELTYPE *mp = &ref_xform[ib][jb][0][0];
        ELTYPE *mp = &(*ref_xform_now)[ib][jb][0][0];
        int idx;
#define loop_body(x) \
idx = jpeg_natural_order[i + (x)]; \
mp[idx] = ((ELTYPE *)pred)[idx] + ((ELTYPE *)tmp)[idx];

#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

        for(int i = 0; i < cpu_save; i += 4)
        {
            loop_body4(0)
            loop_body4(4)
#if MSZ == 16
            loop_body4(8)
            loop_body4(12)
#endif
        }
#undef loop_body
#undef loop_body4
    }
    else
    {
        bcopy(&tmp[0][0], &(*ref_xform_now)[ib][jb][0][0], sizeof(MAT));
    }
}

double
JPEGTCoder::mae_tmp()
{
    long sum = 0;
    int idx;
#define qqabs(p, x) \
		idx = jpeg_natural_order[i + (x)]; \
		t = ((ELTYPE *)tmp)[idx]; \
		t2 = t; \
		t >>= 31; \
		t2 += t; \
		t ^= t2; \
		sum += t;

    for(int i = 0; i < cpu_save; i += 4)
    {
        long t;
        long t2;
        qqabs(i , 0);
        qqabs(i , 1);
        qqabs(i , 2);
        qqabs(i , 3);
    }
    return (double)sum / cpu_save;
}

void
JPEGTCoder::xform(MAT m, QTAB* q)
{
    jpeg_fdct_islow((DCTELEM *)m);
    // drop in a factor of 8 reduction
    ELTYPE *mp = &m[0][0];
    IFAST_MULT_TYPE *FDct_table = q->f_dct;
    int i;
    for(i = 0; i < cpu_save; i += 4)
    {
        mp[jpeg_natural_order[i]] /= FDct_table[jpeg_natural_order[i]];
        mp[jpeg_natural_order[i + 1]] /= FDct_table[jpeg_natural_order[i + 1]];
        mp[jpeg_natural_order[i + 2]] /= FDct_table[jpeg_natural_order[i + 2]];
        mp[jpeg_natural_order[i + 3]] /= FDct_table[jpeg_natural_order[i + 3]];
    }
    for(i = cpu_save; i < 64; ++i)
        mp[jpeg_natural_order[i]] = 0;
}

double
JPEGTCoder::mo_diff_block(int *diff_tmp, int si, int sj, QTAB *qp)
{

#if 0
    // determine if the blocks we need are already
    // decoded.
    int ci = si & ~7;
    int cj = sj & ~7;
    int cib = ci / 8;
    int cjb = cj / 8;
    if(ci == si && cj == sj) // aligned on grid
    {
        if(!decoded[cib][cjb])
        {
            inv_xform((*ref_xform_last)[cib][cjb], ci, cj, qp);
            decoded[cib][cjb] = 1;
        }
    }
    else if(ci == si && cj != sj)
    {
        for(int i = 0; i < 2; ++i)
        {
            if(!decoded[cib][cjb + i])
            {
                inv_xform((*ref_xform_last)[cib][cjb + i], ci, cj + (i * 8), qp);
                decoded[cib][cjb + i] = 1;
            }
        }
    }
    else if(ci != si && cj == sj)
    {
        for(int i = 0; i < 2; ++i)
        {
            if(!decoded[cib + i][cjb])
            {
                inv_xform((*ref_xform_last)[cib + i][cjb], ci + (i * 8), cj, qp);
                decoded[cib + i][cjb] = 1;
            }
        }
    }
    else
    {
        for(int i = 0; i < 2; ++i)
        {
            for(int j = 0; j < 2; ++j)
            {
                if(!decoded[cib + i][cjb + j])
                {
                    inv_xform((*ref_xform_last)[cib + i][cjb + j], ci + (i * 8), cj + (j * 8), qp);
                    decoded[cib + i][cjb + j] = 1;
                }
            }
        }
    }
#endif

    int sum = 0;
    long sum2 = 0;
    // use this is using undecoded frame as reference
    // (not recommended...)
    //t = *diff_tmp++ = tmp[k][x] - g2[x];
    //t = *diff_tmp++ = tmp[k][x] - (g2[x] - BIAS);
#define loop_body(x) \
	t = *diff_tmp++ = (tmp[k][x] + BIAS) - (g2[x]); \
	sum += t; \
	sum2 += sqrs_abs[t];

#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

    for(int k = 0; k < MSZ; ++k)
    {
        gray *g2 = &decoded_frame[si + k][sj];
        int t;
        loop_body4(0)
        loop_body4(4)
#if MSZ == 16
        loop_body4(8)
        loop_body4(12)
#endif
    }
    return variance(MSZ * MSZ, sum, sum2);
#undef loop_body
#undef loop_body4

}

void
JPEGTCoder::diff_block(gray **img, gray **last_img, int si, int sj)
{

    int sum = 0;
    long sum2 = 0;
#define loop_body(x) \
	t = (g[x] - g2[x]) >> 1 ; \
	tmp[k][x] = t;

#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

    for(int k = 0; k < MSZ; ++k)
    {
        gray *g = &img[si + k][sj];
        gray *g2 = &last_img[si + k][sj];
        int t;
        loop_body4(0)
        loop_body4(4)
#if MSZ == 16
        loop_body4(8)
        loop_body4(12)
#endif
    }
    //bvar = variance(MSZ * MSZ, sum, sum2);
#undef loop_body
#undef loop_body4

}

void
JPEGTCoder::motion_compensate(ELTYPE **tmp_pixels, int i, int j, int vi, int vj)
{
    JSAMPLE *Rlimit = Range_limit + CENTERJSAMPLE;
    for(int a = 0; a < MSZ; ++a)
    {
        gray *df = &decoded_frame[i - MV_ZERO + vi + a][j - MV_ZERO + vj];
        gray *dfn = &decoded_frame_now[i + a][j];
        ELTYPE *tp = &tmp_pixels[a][0];
#define loop_body(x) \
			dfn[x] = Rlimit[(df[x] - BIAS + tp[x]) & RANGE_MASK];

        loop_body(0)
        loop_body(1)
        loop_body(2)
        loop_body(3)
        loop_body(4)
        loop_body(5)
        loop_body(6)
        loop_body(7)
#undef loop_body
    }
}
int Total_mo;
int Succ_mo;
void
JPEGTCoder::code_frame(gray **frm, int cols, int rows, int is_ref, const char *polname)
{
    int i, j, k, l;
    int t;
    int ib, jb;
    int hv;
    int low_variance;
    int comp_reset;
    int mocomp;
    MAT tpxs;
    ELTYPE *tpxsp[MSZ];
    for(i = 0; i < MSZ; ++i)
        tpxsp[i] = &tpxs[i][0];
    ELTYPE **tmp_pixels = &tpxsp[0];
    JSAMPLE *Rlimit = Range_limit + CENTERJSAMPLE;

    if((cols & ~STRUNC) != fcols || (rows & ~STRUNC) != frows)
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
    int orows = rows;
    int ocols = cols;
    rows &= ~STRUNC;
    cols &= ~STRUNC;
    ctrl_buf = control_info;
    over_buf = overhead;
    frame_buf = coded_frame;
    frame_vlc_buf = frame_vlc_coeff;
    h_buf = hold_buf;
    ue_buf = use_error_buf;
    outpacker.init();
#ifdef RECORD
    extern Packbits *CTracking;
    CTracking = &this->outpacker;
    Symlist.clear();
#endif
//entropy_code = 0;
    mvi.init();
    mvj.init();
#if 0
    if(entropy_code)
    {
#endif
        //ctrl_encoder.init(ctrl_buf);
        ctrl_packer.init();

        //over_encoder.init(over_buf);
        //frame_vlc.init(frame_vlc_buf);
        fvlc_packer.init();
        use_error_encoder.init();
        hold_encoder.init();
        mocomp_encoder.init();
#if 0
    }
    else
    {
        ctrl_packer.init();
    }
#endif
    dpcm_max_mag.init();
    if(last_frm == 0 || decoded_frame == 0 || still)
        is_ref = 1;
    if(decoded_frame == 0)
        decoded_frame = pgm_allocarray(cols, rows);
    if(decoded_frame_now == 0)
        decoded_frame_now = pgm_allocarray(cols, rows);
    fis_ref = is_ref;
    fcols = cols;
    frows = rows;
    QPolicy *qpp;
    if(!Policy.find(polname, qpp))
        return;
    //EntropyModel& emodel = Lomo;
    //EntropyModel& emodel = XXX_vid;
    start_frame();
    int last_dc_val = 0;
    int *which_dc_val;
    int mocomp_dc_val = 0;
    int dc_val;
    memset(decoded, 0, sizeof(decoded));
    memset(m_motion, 0, sizeof(m_motion));
    memset(m_hv, 1, sizeof(m_hv));
    for(i = 0, ib = 0; i < rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < cols; j += MSZ, ++jb)
        {
            start_block();
            VERB(
                printf("(%d, %d), (%d, %d) ", ib, jb, ib / 2, jb / 2);
            )
            //MAT *rx = &ref_xform[ib][jb];
            double mean_motion = 0;
            int use_error = 1; //1;
            m_hv[ib][jb] = 1;
            if(!is_ref)
            {
#if 0
                compute_motion(frm, last_frm, i, j, noise_clip + 1, mean_motion, &h2);
                compute_motion(frm, last_frm, i, j, noise_clip + 2, mean_motion, &h3);
                compute_motion(frm, last_frm, i, j, noise_clip + 3, mean_motion, &h4);
                compute_motion(frm, decoded_frame, i, j, noise_clip, mean_motion, &h1);
#endif
                compute_motion(frm, decoded_frame, i, j, noise_clip, mean_motion);
                VERB(
                    printf("motion = %f ", mean_motion);
                )
            }
            else
            {
                hold[ib][jb] = 0;
                d_mse[ib][jb] = 0;
                comped[ib][jb] = 0;
                decoded[ib][jb] = 0;
                m_motion[ib][jb] = 0;
                m_hv[ib][jb] = 0;
            }
            DwString blev;
            blev = qpp->names[REFERENCE];
            QTAB *qp;
            if(!QTables.find(blev, qp))
                continue;
            if(ib == 0 && jb == 0)
                ctrl_packer.addbits_raw(qp->id, 5, ctrl_buf);
            if((motion_only || hold[ib][jb]) && !is_ref
                    && mean_motion < mean_motion_thresh)
            {
                VERB(
                    printf("(%d, %d) hold\n", ib, jb);
                )
                hold_encoder.addbit(1, h_buf);
#ifdef GENHIST
                Ts->add_sample(SYM_HOLD, 1);
#endif
                m_motion[ib][jb] = 0;
                goto holdit;
            }
            if(!is_ref)
            {
                hold_encoder.addbit(0, h_buf);
#ifdef GENHIST
                Ts->add_sample(SYM_HOLD, 0);
#endif
                m_motion[ib][jb] = 1;

            }
            hv = 0;
            copy_block(frm, i, j);

            VERB(
                printf("var %f ", bvar);
            )

            low_variance = 0;
            comp_reset = 0;
            mocomp = 0;
            //m_hv[ib][jb] = 1;
            if(bvar < variance_thresh && !is_ref)
            {
                xform(tmp, qp);
                inv_xform(tmp, i, j, qp);
                which_dc_val = &last_dc_val;
                use_error = 0;
                // guarantees we may have to take a couple of
                // frames to notice error settling down again.
                VERB(
                    printf("low variance ");
                )
                low_variance = 1;
                //m_hv[ib][jb] = 0;
                if(mean_motion < mean_motion_thresh)
                    hold[ib][jb] = 1;
                goto quantize;
            }
            else if(use_motion_comp && !is_ref && mean_motion >= mean_motion_thresh &&
                    i >= 8 && i < rows - 8 && j >= 8 && j < cols - 8)
            {
                // try to see if motion comp may help
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
                        printf("vdiff (%f - %f) %f\n", bvar, v, bvar - v);
                    )
#ifdef GENHIST
                    Ts->add_sample(SYM_VEC, vi);
                    Ts->add_sample(SYM_VEC, vj);
#endif
                    mocomp = 1;
                    output_mv(vi, vj);
                    xform(diff_tmp, qp);
                    bcopy(&diff_tmp[0][0], &tmp[0][0], sizeof(MAT));
#if 0
                    ELTYPE *rx = &(*ref_xform_last)[ib][jb][0][0];
                    ELTYPE *rx2 = &(*ref_xform_now)[ib][jb][0][0];
                    ELTYPE *e = &diff_tmp[0][0];
                    for(int a = 0; a < 64; ++a)
                        *rx2++ = *rx++ + *e++;
#endif
                    jpeg_idct_islow((JCOEFPTR)&diff_tmp[0][0], tmp_pixels, 0, qp, 0);
#if 0
                    for(int a = 0; a < 8; ++a)
                    {
                        for(int b = 0; b < 8; ++b)
                        {
                            int t = decoded_frame[i - MV_ZERO + vi + a][j - MV_ZERO + vj + b];
                            decoded_frame_now[i + a][j + b] = Rlimit[(t - BIAS + tmp_pixels[a][b]) & RANGE_MASK];
                        }
                    }
#endif
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
            {
                // use zero vector
                MAT diff_tmp;
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
            xform(tmp, qp);
            inv_xform(tmp, i, j, qp);
            use_error = 0;
            which_dc_val = &last_dc_val;

quantize:
            if(!is_ref)
            {
                mocomp_encoder.addbit(mocomp, over_buf);
                if(!mocomp)
                {
                    m_hv[ib][jb]= use_error;
                    use_error_encoder.addbit(use_error, ue_buf);
                }
#ifdef GENHIST
                Ts->add_sample(SYM_USE_ERROR, use_error);
#endif
                VERB(
                    printf("use err: %d\n", use_error);
                )
            }
            dc_val = *which_dc_val;
            encode_one_block ((JCOEF *)tmp, dc_val,
                              Dc_derived_tbls_c[0], Ac_derived_tbls_c[0], outpacker, frame_buf, cpu_save, coeff_tweak);
            *which_dc_val = dc_val;
            goto done_block;

holdit:
            {

                // block is on hold, must copy it from previous
                // frame
                int r, c;
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
#if 0
    if(entropy_code)
    {
#endif
        //ctrl_encoder.finish(ctrl_buf);
        ctrl_packer.flush(ctrl_buf);

        //over_encoder.finish(over_buf);
        //frame_vlc.finish(frame_vlc_buf);
        fvlc_packer.flush(frame_vlc_buf);
        use_error_encoder.flush(ue_buf);
        hold_encoder.flush(h_buf);
        mocomp_encoder.flush(over_buf);
#if 0
    }
    else
    {
        ctrl_packer.flush(ctrl_buf);
    }
#endif
    //dpcm_max_mag.flush(over_buf);
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
