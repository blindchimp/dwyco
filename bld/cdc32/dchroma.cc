
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dchroma.cc 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "chroma.h"
#include "dchroma.h"
#include "imgmisc.h"
#include "jqtab.h"
#include "jdct.h"
void oopanic(const char *);
#if __GNUC_MINOR__ >= 7 || defined(__linux__)
#include <netinet/in.h>
#endif
#ifdef _Windows
#include <winsock.h>
#endif
#ifdef LINUX
#include <arpa/inet.h>
#endif

JPEGTDecoderChroma::JPEGTDecoderChroma(int plane, JPEGTDecoder *l)
{
    which_plane = plane;
    luma = l;
}

JPEGTDecoderChroma::~JPEGTDecoderChroma()
{
}

unsigned short
JPEGTDecoderChroma::get_chroma_info(DWBYTE *frm, unsigned short *p, int& has_chroma)
{
    // p points to the chroma offset
    // depending on which plane we are, we have
    // to fetch the actual offset to the
    // chroma data.
    if(ntohs(*p) == 0)
    {
        has_chroma = 0;
        return 1;
    }
    else
        has_chroma = 1;
    switch(which_plane)
    {
    case 0:
        return ntohs(*p) + CHROMA_HEADER_SZ;
    case 1:
        return ntohs(*(unsigned short *)(frm + ntohs(*p)));
    }
    oopanic("bad chroma plane");
    // NOTREACHED
    return 0;
}



void
JPEGTDecoderChroma::decode_frame(DWBYTE* ctrl, DWBYTE* overhead, BITBUFT* img,
                                 BITBUFT *vlc, BITBUFT *hold_buf, BITBUFT *use_error_buf,
                                 void *&app_img, int icols, int irows, int is_ref)
{

    // note: ignore all but "img", since all the other info
    // is only valid for the luma plane.
    // icols and irows is going to be twice the actual
    // size we need here, since they were for the luma
    // plane as well.

    int i, j;
    int ib, jb;

    extern JSAMPLE *Range_limit;
    JSAMPLE *Rlimit = Range_limit + CENTERJSAMPLE;
    MAT tpxs;
    int *tpxsp[MSZ];
    for(i = 0; i < MSZ; ++i)
        tpxsp[i] = &tpxs[i][0];
    int **tmp_pixels = &tpxsp[0];

    compute_subsampled_region(icols, irows, 1, (get_sampling() == SAMP411) ? 4 : 2);
    if(ri_cols != (icols & ~STRUNC) || ri_rows != (irows & ~STRUNC))
    {
        // image has changed dimension, reset
        // everything.
        if(ref_img)
            dealloc_refimg();
        if(last_frame)
            pgm_freearray(last_frame, ri_rows);
        ref_img = 0;
        last_frame = 0;
    }
    if(ref_img == 0)
    {
        ri_rows = irows & ~STRUNC;
        ri_cols = icols & ~STRUNC;
        ref_img = alloc_appimg(ri_cols / MSZ, ri_rows / MSZ);
    }
    if(!is_ref)
    {
        if(last_frame == 0)
        {
            last_frame = (gray **)ref_img;
            ref_img = alloc_appimg(ri_cols / MSZ, ri_rows / MSZ);
        }
        else
        {
            gray **tmp = (gray **)ref_img;
            ref_img = last_frame;
            last_frame = tmp;
        }
    }
    junpacker.init(img);
#ifdef RECORD
    extern JDUnpackbits *DTracking;
    DTracking = &junpacker;
#endif
    dpcm_max_mag.init();
    start_frame();
    QTAB *qp;
    if(!still)
    {
        if(!QidTables.find(luma->qid + Q_JPEG_CHROMA_OFF, qp))
            return;
#if 0
        printf("qidr %d\n", luma->qid + Q_JPEG_CHROMA_OFFSET);
#endif
    }
    int last_dc_val = 0;
    for(i = 0, ib = 0; i < ri_rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < ri_cols; j += MSZ, ++jb)
        {
            start_block();
            int hold;
            if(!is_ref)
            {
                if(get_sampling() == SAMP422)
                    hold = luma->b_hold[2 * ib][2 * jb] &&
                           luma->b_hold[2 * ib + 1][2 * jb] &&
                           luma->b_hold[2 * ib][2 * jb + 1] &&
                           luma->b_hold[2 * ib + 1][2 * jb + 1];
                else if(get_sampling() == SAMP411)
                {
                    for(int il = 0; il < 4; ++il)
                        for(int jl = 0; jl < 4; ++jl)
                        {
                            if(!luma->b_hold[4 * ib + il][4 * jb + jl])
                            {
                                hold = 0;
                                goto out;
                            }
                        }
                    hold = 1;
out:
                    ;
                }
            }
            if(is_ref || !hold)
            {
                int use_error;
                if(!is_ref || still)
                {
                    if(!still)
                    {
#if 0
                        int mocomp = mocomp_decoder.getbit(over);
                        if(mocomp)
                        {
                            MAT tmp;
                            memset(tmp, 0, sizeof(tmp));
                            decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 1);
                            // note: clipping done in convert probably screws things
                            jpeg_idct_islow((JCOEFPTR)&tmp[0][0], tmp_pixels, 0, qp, 0);
                            int vi, vj;
                            get_mv(vi, vj, vlc);
                            int a, b;
                            int ioff = i + vi - MV_ZERO;
                            int joff = j + vj - MV_ZERO;
                            for(a = 0; a < 8; ++a)
                                for(b = 0; b < 8; ++b)
                                {
                                    ((gray **)ref_img)[i + a][j + b] = Rlimit[(tmp_pixels[a][b] + last_frame[a + ioff][b + joff] - BIAS) & RANGE_MASK];
                                }
                            continue;
                        }
                        else
#endif
                        {
                            if(get_sampling() == SAMP422)
                            {
                                use_error = luma->b_ue[2 * ib][2 * jb] ||
                                            luma->b_ue[2 * ib + 1][2 * jb] ||
                                            luma->b_ue[2 * ib][2 * jb + 1] ||
                                            luma->b_ue[2 * ib + 1][2 * jb + 1];
                            }
                            else if(get_sampling() == SAMP411)
                            {
                                for(int il = 0; il < 4; ++il)
                                    for(int jl = 0; jl < 4; ++jl)
                                    {
                                        if(luma->b_ue[4 * ib + il][4 * jb + jl])
                                        {
                                            use_error = 1;
                                            goto out2;
                                        }
                                    }
                                use_error = 0;
out2:
                                ;
                            }
                            else
                            {
                                // sampling isn't set right,just bail out on the decode
                                return;
                            }

                            if(use_error)
                            {
                                MAT tmp;
                                memset(tmp, 0, sizeof(tmp));
                                decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 1, tweak);
                                // note: clipping done in convert probably screws things
                                jpeg_idct_islow((JCOEFPTR)&tmp[0][0], tmp_pixels, 0, qp, 0);
                                int a, b;
                                for(a = 0; a < 8; ++a)
                                    for(b = 0; b < 8; ++b)
                                    {
                                        ((gray **)ref_img)[i + a][j + b] = Rlimit[(tmp_pixels[a][b] + last_frame[i + a][j + b] - BIAS) & RANGE_MASK];
                                    }
                                continue;
                            }
                            else
                            {
                                MAT tmp;
                                memset(tmp, 0, sizeof(tmp));
                                decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 1, tweak);
                                // note: clipping done in convert probably screws things
                                convert(tmp, app_img, i, j, qp);
                                continue;
                            }
                        }
                    }
                }

//***********
//***********
                MAT tmp;
                memset(tmp, 0, sizeof(tmp));
                decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 1, tweak);

                // note: jpeg idct does all clipping and conversion in one shot
                // so it is called in "convert"
                inv_xform(tmp);
                convert(tmp, app_img, i, j, qp);
            }
            else
            {
                // block is on hold, must copy it from previous
                // frame
                int r;
                for(r = i; r < i + 8; ++r)
                {
                    *(dwINT32 *)&((gray **)ref_img)[r][j] = *(dwINT32 *)&last_frame[r][j] ;
                    *((dwINT32 *)&((gray **)ref_img)[r][j] + 1) = *((dwINT32 *)&last_frame[r][j] + 1);
                }
            }
            finish_block();
        }
        finish_line();
    }
    finish_frame();
}
