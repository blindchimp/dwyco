#ifdef DWYCO_DCT_CODER
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tcode.cc 1.9 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "tcode.h"
#include "pbmcfg.h"
#include "matcom.h"
#include "nyb.h"
#include "qtab.h"
#include "qpol.h"
#include "macs.h"
#include "imgmisc.h"
#include "statfun.h"
#include "sqrs.h"
#include "tpgmdec.h"
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"
#ifdef _Windows
#include <WinSock2.h>
#endif
#include "pred.h"
#if __GNUC_MINOR__ >= 7 || defined(__linux__)
#include <netinet/in.h>
#endif
#ifdef LINUX
#include <arpa/inet.h>
#endif

#if 0
#define VERB(x) if(Verbose) {x}
//#define VERB(x)
int Verbose = 0;
#endif

void fht_row(ELTYPE *, ELTYPE *);
void fht_col(ELTYPE *, ELTYPE *);

TCoder::TCoder()
{
    //this->cols = cols;
    //this->rows = rows;
    last_frm = 0;
    //ref_xform = new XFRM;
    memset(hold, 0, sizeof(hold));
    memset(ref_xform, 0, sizeof(ref_xform));
    memset(d_mse, 0, sizeof(d_mse));
    // note: this is all very dicey in 16-bitsville.
    // we keep the sizes under 64k, and a reasonable
    // size for expected number of bytes generated.
    // unfortunately, we have no way to get a good
    // upper bound on amount of buffer space needed when
    // using entropy coding, and
    // using checking on every byte output is expensive,
    // so we just wing it for now.
    //
    coded_frame = new BITBUFT[BLKR * BLKC * MSZ * MSZ / sizeof(BITBUFT)];
    frame_vlc_coeff = new BITBUFT[BLKR * BLKC * MSZ * MSZ / sizeof(BITBUFT)];
#if 0
    control_info = new DWBYTE[BLKR * BLKC];
    overhead = new DWBYTE[BLKR * BLKC * (NQUADS * NQUADS + 1)]; // +1 since mean gets its own
#endif
    control_info = new BITBUFT[BLKR * BLKC];
    overhead = new BITBUFT[BLKR * BLKC * (NQUADS * NQUADS + 1)]; // +1 since mean gets its own
    hold_buf = new BITBUFT[BLKR * BLKC];
    use_error_buf = new BITBUFT[BLKR * BLKC];
    mse_hold_thresh = 1;
    mse_refine_thresh = 1;
    stdev_motion_thresh = 5;
    mean_motion_thresh = 1;
    variance_thresh = 64;
    noise_clip = 4;
    motion_only = 0;
    entropy_code = 0;
    still = 0;
    fis_ref = 0;
    fcols = 0;
    frows = 0;
    bvar = 0;
    last_was_ref = 1;
#ifdef GENHIST
    Ts->add_sym(SYM_HOLD, "hold", 0, 1, 2);
    Ts->add_sym(SYM_QID, "qid", 0, 31, 32);
    Ts->add_sym(SYM_USE_ERROR, "use_error", 0, 1, 2);
    //Ts->add_sym(SYM_REF_DC_BITS, "ref_dc", -1024, 1023, 2048);
    Ts->add_sym(SYM_NREF_DC_BITS, "nref_dc_bits", -32, 31, 64);
    Ts->add_sym(SYM_REF_AC_BITS, "ref_ac_bits", -32, 31, 64);
    Ts->add_sym(SYM_NREF_AC_BITS, "nref_ac_bits", -32, 31, 64);
    Ts->add_sym(SYM_VEC, "vec", 0, 15, 16);
#if 0
    for(int blev = 1; blev < 8; ++blev)
    {
        char bn[200];
        sprintf(bn, "ref_ac_%d", blev);
        Ts->add_sym(SYM_REF_AC(blev, log2mag), bn,
                    0, (1 << blev) - 1, 1 << blev);

        sprintf(bn, "nref_ac_%d", blev);
        Ts->add_sym(SYM_NREF_AC(blev, log2mag), bn,
                    0, (1 << blev) - 1, 1 << blev);
    }
#endif
    Ts->add_sym(SYM_REF_AC(1, __bar__), "ref_ac", 0, 255, 256);
    Ts->add_sym(SYM_NREF_AC(1, __bar__), "nref_ac", 0, 255, 256);
#endif
}

TCoder::~TCoder()
{
    delete [] coded_frame;
    delete [] control_info;
    delete [] overhead;
    delete [] frame_vlc_coeff;
    delete [] hold_buf;
    delete [] use_error_buf;
    if(last_frm)
        pgm_freearray(last_frm, frows);
}

void
TCoder::reset()
{
    if(last_frm)
        pgm_freearray(last_frm, frows);
    last_frm = 0;
    frows = 0;
    fcols = 0;
}

void
TCoder::set_special_parms(double mae_hold, double mae_switch, double mad,
                          int temp_comp, int noise, int ent_code, int, int, unsigned int)
{
    mse_hold_thresh = mae_hold;
    mse_refine_thresh = mae_switch;
    mean_motion_thresh = mad;
    motion_only = temp_comp;
    noise_clip = noise;
    entropy_code = ent_code;
}

#define MOTION_DONT_USE_STDEV
#define MOTION_USE_C

#if defined(__GNUG__)
#define MOTION_USE_C
#endif

#ifndef MOTION_USE_C
#pragma option -rd
#pragma option -Od
#endif
double
compute_motion(gray **f1, gray **f0, int ssi, int sj, int clip,
               double& mean_out)
{
#ifdef MOTION_USE_C
    long sum = 0;
#else
    int sum = 0;
#endif
    long sum2 = 0;
    int cnt = 0;
#ifdef MOTION_USE_C
    long tmp;
    long t2;

#ifdef MOTION_DONT_USE_STDEV
#define sumq ;
#else
#define sumq sum2 += sqrs[tmp];
#endif

#define mdo_loop_body \
		tmp = *r0++ - *r1++; \
		t2 = tmp; \
		tmp >>= 31; \
		t2 += tmp; \
		tmp ^= t2; \
		if(tmp > clip) \
		{ \
		sum += tmp - clip; \
		sumq \
		++cnt; \
		}
#define mdo_loop4(__not_used, __not_used2)  \
	mdo_loop_body \
	mdo_loop_body \
	mdo_loop_body \
	mdo_loop_body
#define mdo_init_line
#define mdo_end_line
#elif defined(__LARGE__)

#define mdo_init_line \
asm {\
	.386p ;\
	push ds ;\
	les si, r0 ;\
	lds di, r1 ;\
}
#define mdo_end_line \
asm { \
	.386p ; \
	pop ds ; \
}
// 16-bit version, pentium optimized
#define two_pix(a, lab) \
asm { \
	xor ax, ax ; \
	xor cx, cx ; \
	xor bx, bx ; \
	xor dx, dx ; \
	mov al, es:[si + a] ; \
	mov cl, es:[si + 4 + a] ; \
	mov bl, [di + a] ; \
	mov dl, [di + 4 + a] ; \
	sub ax, bx ; \
	sub cx, dx ; \
	mov bx, ax ; \
	mov dx, cx ; \
	sar bx, 15 ; \
	sar dx, 15 ; \
	add ax, bx ; \
	add cx, dx ; \
	xor ax, bx ; \
	xor cx, dx ; \
	mov dx, clip ; \
	sub ax, dx ; \
	jle lab ; \
	inc word ptr cnt ; \
	add sum, ax ; \
} \
lab: ; \
asm { \
	sub cx, dx ; \
	jle lab##_2 ; \
	inc word ptr cnt ; \
	add sum, cx ;  \
} \
lab##_2: ;

#define mdo_loop4(a, lab) \
	two_pix(a, _a##lab) \
	two_pix(a + 1, _b##lab)

#elif defined(__BORLANDC__)

#define mdo_init_line \
asm {\
	.486p ;\
	mov eax, ssi ; \
	mov edx, sj ; \
	add eax, i ; \
	mov ebx, f0 ; \
	mov ebx, [ebx + eax*4] ; \
	lea esi, [ebx + edx] ;\
	mov ebx, f1 ; \
	mov ebx, [ebx + eax*4] ; \
	lea edi, [ebx + edx] ;\
}
#define mdo_end_line \
asm { \
	.486p ; \
}
// 32-bit version, pentium optimized
#define two_pix(a, lab) \
asm { \
	xor eax, eax ; \
	xor ecx, ecx ; \
	xor ebx, ebx ; \
	xor edx, edx ; \
	mov al, [esi + a] ; \
	mov cl, [esi + 4 + a] ; \
	mov bl, [edi + a] ; \
	mov dl, [edi + 4 + a] ; \
	sub eax, ebx ; \
	sub ecx, edx ; \
	mov ebx, eax ; \
	mov edx, ecx ; \
	sar ebx, 31 ; \
	sar edx, 31 ; \
	add eax, ebx ; \
	add ecx, edx ; \
	xor eax, ebx ; \
	xor ecx, edx ; \
	mov edx, clip ; \
	sub eax, edx ; \
	jle lab ; \
	inc dword ptr cnt ; \
	add sum, eax ; \
} \
lab: ; \
asm { \
	sub ecx, edx ; \
	jle lab##_2 ; \
	inc dword ptr cnt ; \
	add sum, ecx ;  \
} \
lab##_2: ;

#define mdo_loop4(a, lab) \
	two_pix(a, _a##lab) \
	two_pix(a + 1, _b##lab)
#else
#error ooops, need to define MOTION_USE_C??
#endif

    unsigned char *r0;
    unsigned char *r1;
    for(int i = 0; i < MSZ; ++i)
    {
#if defined(MOTION_USE_C) || defined(__LARGE__)
        r0 = &f0[ssi + i][sj];
        r1 = &f1[ssi + i][sj];
#endif
        mdo_init_line
        mdo_loop4(0, foo)
        mdo_loop4(2, bar)
#if MSZ == 16
        mdo_loop4
        mdo_loop4
#endif
        mdo_end_line
    }
    if(cnt == 0)
        mean_out = 0;
    else
        mean_out = (double)sum / cnt;
#if 0
    if(h)
        h->add_sample(mean_out);
#endif
#ifndef MOTION_DONT_USE_STDEV
    return stdev(MSZ * MSZ, sum, sum2);
#else
    return 0;
#endif
#undef mdo_loop_body
#undef mdo_loop4
#undef mdo_init_line
}
#ifndef MOTION_USE_C
#pragma option -rd.
#pragma option -O.
#endif

void
TCoder::copy_block(gray **img, int si, int sj)
{

    int sum = 0;
    long sum2 = 0;
#define loop_body(x) \
	t = g[x]; \
	sum += t; \
	sum2 += sqrs[t]; \
	tmp[k][x] = t - BIAS;

#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

    for(int k = 0; k < MSZ; ++k)
    {
        gray *g = &img[si + k][sj];
        int t;
        loop_body4(0)
        loop_body4(4)
#if MSZ == 16
        loop_body4(8)
        loop_body4(12)
#endif
    }
    bvar = variance(MSZ * MSZ, sum, sum2);
#undef loop_body
#undef loop_body4

}

void
TCoder::xform(MAT m, QTAB *)
{
    jpeg_fdct_ifast((DCTELEM *)m);
    // drop in a factor of 8 reduction
    ELTYPE *mp = &m[0][0];
    IFAST_MULT_TYPE *FDct_table = QTAB::JustDctScales.f_dct;
    for(int i = 0; i < MSZ * MSZ; i += 4)
    {
        *mp++ /= FDct_table[i];
        *mp++ /= FDct_table[i + 1];
        *mp++ /= FDct_table[i + 2];
        *mp++ /= FDct_table[i + 3];
    }
}

void
TCoder::inv_xform(MAT m, int, int, QTAB *)
{
}

// note: assumes 8x8, use mae_vec (general nxn)
// otherwise...
double
TCoder::mae_tmp()
{
    long sum = 0;
#define qqabs(p, k) \
		t = tmp[p][k]; \
		t2 = t; \
		t >>= 31; \
		t2 += t; \
		t ^= t2; \
		sum += t;
    int iter = (MSZ * MSZ) >> 3;
    for(int i = 0; i < iter; ++i)
    {
        long t;
        long t2;
        qqabs(i , 0);
        qqabs(i , 1);
        qqabs(i , 2);
        qqabs(i , 3);
        qqabs(i , 4);
        qqabs(i , 5);
        qqabs(i , 6);
        qqabs(i , 7);
    }
    return (double)sum / (MSZ * MSZ);
}

double
TCoder::predict(int ib, int jb)
{
    bcopy(&tmp[0][0], &xform_res[0][0], sizeof(MAT));
    MAT& r = ref_xform[ib][jb];
#define loop_body(x) \
	tp = PREDICT(r[k][x]); \
	pred[k][x] = tp; \
	tmp[k][x] -= tp;
#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

    for(int k = 0; k < MSZ; ++k)
    {
        ELTYPE tp;
        loop_body4(0)
        loop_body4(4)
#if MSZ == 16
        loop_body4(8)
        loop_body4(12)
#endif
    }
#undef loop_body
#undef loop_body4
    double mean;
    double d = mae_tmp();
    return d;
}

void
TCoder::predict_recon(int is_ref, int ib, int jb, QTAB& q)
{
    if(!is_ref)
    {
        if(q.reconfun != 0)
        {
            // get untouched coeffs
            bcopy(&pred[0][0], &ref_xform[ib][jb][0][0], sizeof(MAT));
            QTAB_RECONFUNP f = q.reconfun;
            (this->*f)(ib, jb);
            return;
        }
#define loop_body(x) \
ref_xform[ib][jb][k][x] = pred[k][x] + tmp[k][x];

#define loop_body4(x) \
	loop_body(x) \
	loop_body(x + 1) \
	loop_body(x + 2) \
	loop_body(x + 3)

        for(int k = 0; k < MSZ; ++k)
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
        bcopy(&tmp[0][0], &ref_xform[ib][jb][0][0], sizeof(MAT));
        //bcopy(tmp, Ref_xform2[ib][jb], sizeof(tmp));
    }
}



DWBYTE *
TCoder::final_package(int &len_out)
{
    int len;

    int len_frame = (DWBYTE *)frame_buf - (DWBYTE *)coded_frame;
    int len_ctrl = (DWBYTE *)ctrl_buf - (DWBYTE *)control_info;
    int len_over = (DWBYTE *)over_buf - (DWBYTE *)overhead;
    int len_vlc = (DWBYTE *)frame_vlc_buf - (DWBYTE *)frame_vlc_coeff;
    int len_hold = (DWBYTE *)h_buf - (DWBYTE *)hold_buf;
    int len_use_error = (DWBYTE *)ue_buf - (DWBYTE *)use_error_buf;

#if 0
    //
    // if it is entropy coded, add some slop (apparently the
    // decoder needs to run over a few bits to get the
    // last token, i'm not sure why, but i'm too lazy to
    // figure it out at the moment.
    //
    if(entropy_code)
    {
        len_ctrl += 4;
        len_over += 4;
        len_vlc += 4;
    }
#endif
    // need to put in the size of the frame in blocks as
    // well as whether it is a reference frame.
    // should this control info go out in another
    // stream?
#define HEADER_SZ 20
    len = HEADER_SZ + len_frame + len_ctrl +
          len_over + len_vlc + len_use_error + len_hold; // overhead bytes for offsets to data segments

    DWBYTE *buf = new DWBYTE[len];

    DWBYTE *nb = buf;
    // format of header is:
    // byte
    // 0    u_char options (bitmask)
    // 1    u_char 0
    // 2-3  short # image block cols
    // 4-5  short # image blocks rows
    // 6-7  short offset to image frame data (offset from beginning of packet)
    // 8-9  short offset to overhead
    // 10-11 short offset to vector info
    // 12-13 short offset to hold bits
    // 14-15 short offset to use-vector bits
    // 16-17 short offset to color info
    // 18		u_char timecode
    // 19		u_char 0
    *nb = (!!fis_ref << 7) | (!!entropy_code << 6) | (!!still << 5);
    *(nb + 1) = 0;
    nb += sizeof(unsigned short);
    *(unsigned short *)nb = htons(fcols / MSZ);
    nb += sizeof(unsigned short);
    *(unsigned short *)nb = htons(frows / MSZ);
    nb += sizeof(unsigned short);

    // offset to image frame data
    unsigned short us = len_ctrl + len_over + HEADER_SZ;
    *(unsigned short *)nb = htons(us);
    nb += sizeof(unsigned short);

    // offset to overhead info
    us = len_ctrl + HEADER_SZ;
    *(unsigned short *)nb = htons(us);
    nb += sizeof(unsigned short);

    // offset to vlc info
    us = len_ctrl + len_over + len_frame + HEADER_SZ;
    *(unsigned short *)nb = htons(us);
    nb += sizeof(unsigned short);

    // offset to hold
    us = len_ctrl + len_over + len_frame + len_vlc + HEADER_SZ;
    *(unsigned short *)nb = htons(us);
    nb += sizeof(unsigned short);

    // offset to error
    us = len_ctrl + len_over + len_frame + len_vlc + len_hold + HEADER_SZ;
    *(unsigned short *)nb = htons(us);
    nb += sizeof(unsigned short);

    // offset to color info
    *(unsigned short *)nb = 0;
    nb += sizeof(unsigned short);

    // misc timecode, etc.
    *(unsigned short *)nb = 0;
    nb += sizeof(unsigned short);

    bcopy(control_info, nb, len_ctrl);
    nb += len_ctrl;
    bcopy(overhead, nb, len_over);
    nb += len_over;
    bcopy(coded_frame, nb, len_frame);
    nb += len_frame;
    bcopy(frame_vlc_coeff, nb, len_vlc);
    nb += len_vlc;
    bcopy(hold_buf, nb, len_hold);
    nb += len_hold;
    bcopy(use_error_buf, nb, len_use_error);

    len_out = len;
    return buf;
}

void
TCoder::code_frame(gray **frm, int cols, int rows, int is_ref, const char *polname)
{
#ifdef DWYCO_CODEC
    int i, j, k, l;
    int t;
    int ib, jb;
    int hv;

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
    if(entropy_code)
    {
        ctrl_encoder.init(ctrl_buf);
        over_encoder.init(over_buf);
        //frame_vlc.init(frame_vlc_buf);
        fvlc_packer.init();
        use_error_encoder.init();
        hold_encoder.init();
    }
    else
    {
        ctrl_packer.init();
    }
    dpcm_max_mag.init();
    if(last_frm == 0 || still)
        is_ref = 1;
    fis_ref = is_ref;
    fcols = cols;
    frows = rows;
    QPolicy *qpp;
    if(!Policy.find(polname, qpp))
        return;
    EntropyModel& emodel = Lomo;
    //EntropyModel& emodel = XXX_vid;
    start_frame();
    for(i = 0, ib = 0; i < rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < cols; j += MSZ, ++jb)
        {
            start_block();
            //MAT *rx = &ref_xform[ib][jb];
            double sdev_motion = 0;
            double mean_motion = 0;
            int use_error = 1;
            if(!is_ref)
            {
                sdev_motion = compute_motion(frm, last_frm, i, j, noise_clip, mean_motion);
                VERB(
                    printf("motion = %f\n", mean_motion);
                )
            }
            else
            {
                hold[ib][jb] = 0;
                d_mse[ib][jb] = 0;
            }
            char *blev;
            if((motion_only || hold[ib][jb]) && !is_ref
#ifndef MOTION_DONT_USE_STDEV
                    && sdev_motion < stdev_motion_thresh
#endif
                    && mean_motion < mean_motion_thresh)
            {
                VERB(
                    printf("(%d, %d) hold\n", ib, jb);
                )
                if(entropy_code)
                    //ctrl_encoder.encode_a_symbol(1, emodel[SYM_HOLD], ctrl_buf);
                    hold_encoder.addbit(1, h_buf);
                else
                    ctrl_packer.addbits_raw(1, 1, ctrl_buf);
#ifdef GENHIST
                Ts->add_sample(SYM_HOLD, 1);
#endif
#if 0
                *ctrl_buf++ = HOLD_BLOCK;
#endif
                goto holdit;
            }
            if(!is_ref)
            {
                if(entropy_code)
                    //ctrl_encoder.encode_a_symbol(0, emodel[SYM_HOLD], ctrl_buf);
                    hold_encoder.addbit(0, h_buf);
                else
                    ctrl_packer.addbits_raw(0, 1, ctrl_buf);
#ifdef GENHIST
                Ts->add_sample(SYM_HOLD, 0);
#endif
            }
            copy_block(frm, i, j);
            xform(tmp, 0);

            VERB(
                printf("var %f ", bvar);
            )
            hv = 0;
            if(bvar > variance_thresh)
                hv = HV_OFFSET;
            if(is_ref)
            {
// note: no HV mode for ref frames (might want to correct this)
                if(still)
                    blev = qpp->names[REFERENCE + hv];
                else
                    blev = qpp->names[REFERENCE];
                VERB(
                    printf("(%d, %d)\n", ib, jb);
                )
            }
            else
            {
                double d = predict(ib, jb);

                if(d < mse_hold_thresh)
                {
                    blev = qpp->names[MSE_CLOSE + hv];
                    VERB(
                        printf("mse close ");
                    )
                    d_mse[ib][jb] = d;
                    hold[ib][jb] = 1;
                }
                else
                {
                    hold[ib][jb] = 0;
                    double df = d - d_mse[ib][jb];
                    VERB(
                        printf("dmse %f ", df);
                    )
                    if(!last_was_ref && df > 5) // error is increasing, don't bother trying to quantize it.
                    {
                        use_error = 0;
                        blev = qpp->names[REFERENCE + hv];
                        d_mse[ib][jb] = d;
                        VERB(
                            printf("anti-refine\n");
                        )
                        goto quantize;
                    }
                    else
                        use_error = 1;
                    df = qabs(df);
                    if(df < mse_refine_thresh)
                    {
                        VERB(
                            printf("stopped refining ");
                        )
                        // picture isn't refining anymore,
                        // switch to new quantization table
                        // unless there is no motion
                        if(
#ifndef MOTION_DONT_USE_STDEV
                            sdev_motion < stdev_motion_thresh &&
#endif
                            mean_motion < mean_motion_thresh)
                        {
                            VERB(
                                printf("no motion ");
                            )
                            blev = qpp->names[STOPPED_REFINING_NO_MOTION + hv];
                            hold[ib][jb] = 1;
                        }
                        else
                        {
                            VERB(
                                printf("more refine ");
                            )
                            blev = qpp->names[STOPPED_REFINING_MOTION + hv];
                        }
                    }
                    else
                    {
                        VERB(
                            printf("still refining ");
                        )
                        if(
#ifndef MOTION_DONT_USE_STDEV
                            sdev_motion < stdev_motion_thresh &&
#endif
                            mean_motion < mean_motion_thresh)
                        {
                            VERB(
                                printf("no motion ");
                            )
                            blev = qpp->names[REFINING_NO_MOTION + hv];
                        }
                        else
                        {
                            VERB(
                                printf("motion ");
                            )
                            blev = qpp->names[REFINING_MOTION + hv];
                        }
                    }
                    d_mse[ib][jb] = d;
                }
                VERB(
                    printf("(%d, %d) motion %f, mse %d\n", ib, jb, mean_motion, d);
                )
            }
quantize:
            QTAB *qp;
            if(QTables.find(blev, qp))
            {
#if 0
                *ctrl_buf++ = qp->id;
#else
                if(!is_ref || (is_ref && ib == 0 && jb == 0) || still)
                {
#if 0
                    printf("qid %d\n", qp->id);
#endif
                    VERB(
                        printf("q: %s\n", blev);
                    )
                    if(entropy_code)
                        ctrl_encoder.encode_a_symbol(qp->id, emodel[SYM_QID], ctrl_buf);
                    else
                        ctrl_packer.addbits_raw(qp->id, 5, ctrl_buf);
#ifdef GENHIST
                    Ts->add_sample(SYM_QID, qp->id);
#endif
                }
#endif
                if(!is_ref)
                {
                    if(entropy_code)
                    {
                        //ctrl_encoder.encode_a_symbol(use_error, emodel[SYM_USE_ERROR], ctrl_buf);
                        use_error_encoder.addbit(use_error, ue_buf);
                    }
                    else
                        ctrl_packer.addbits_raw(use_error, 1, ctrl_buf);
#ifdef GENHIST
                    Ts->add_sample(SYM_USE_ERROR, use_error);
#endif
                    VERB(
                        printf("use err: %d\n", use_error);
                    )
                    if(!use_error)
                    {
                        // restore unpredicted xform coefs
                        bcopy(&xform_res[0][0], &tmp[0][0], sizeof(MAT));
                    }
                }
                bitalloc_quantize(is_ref, use_error, *qp,
#if 0
                                  ctrl_buf, frame_buf, over_buf,
#endif
                                  outpacker, dpcm_max_mag, emodel);
                // recreate reference xform, to keep in sync with receiver
                if(!still)
                    predict_recon(is_ref || !use_error, ib, jb, *qp);
            }
            else
            {
                bzero(&ref_xform[ib][jb][0][0], sizeof(MAT));
            }

#if 0
            VERB(
                double d = mae_vec(&tmp[0][0], MSZ * MSZ, mean);
                printf("mean, mse (dpcm-q)= %f, %f\n", mean, d);
            )
#endif

holdit:
            ;
            finish_block();
        }
        finish_line();
    }
    outpacker.flush(frame_buf);
    if(entropy_code)
    {
        ctrl_encoder.finish(ctrl_buf);
        over_encoder.finish(over_buf);
        //frame_vlc.finish(frame_vlc_buf);
        fvlc_packer.flush(frame_vlc_buf);
        use_error_encoder.flush(ue_buf);
        hold_encoder.flush(h_buf);
    }
    else
    {
        ctrl_packer.flush(ctrl_buf);
    }
    dpcm_max_mag.flush(over_buf);
    if(last_frm != 0)
        pgm_freearray(last_frm, frows);
    last_frm = frm;
    last_was_ref = is_ref;
    finish_frame();
#endif
}
#endif
