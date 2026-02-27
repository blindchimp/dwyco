
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jtdecode.cc 1.15 1999/01/10 16:09:34 dwight Checkpoint $
 */
#include "tcode.h"
#include "jtdecode.h"
#include "qtab.h"
#include "nyb.h"
#include "packbits.h"
#include "mo.h"
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"
#include "jdhuff.h"
#include "pred.h"

extern JSAMPLE *Range_limit;

JPEGTDecoder::JPEGTDecoder()
{
    last_frame = 0;
}
JPEGTDecoder::~JPEGTDecoder()
{
    if(last_frame != 0)
        pgm_freearray(last_frame, ri_rows);
}

int
JPEGTDecoder::decode_vec(BITBUFT*& vlc)
{
    int i;

    for(i = 0; i < 4; ++i) // 0, 1, 2, 3
    {
        if(fvlc_unpacker.getbit_raw(vlc))
        {
            if(i == 0)
                return 0;
            if(fvlc_unpacker.getbit_raw(vlc))
                return -i;
            return i;
        }
    }
    switch(fvlc_unpacker.getbits_raw(3, vlc)) // 4, 5, 6
    {
    case 6:
        return 4;
    case 5:
        if(fvlc_unpacker.getbit_raw(vlc))
            return -5;
        return 5;
    case 4:
        if(fvlc_unpacker.getbit_raw(vlc))
            return -6;
        return 6;
    case 7:
        return -4;
    case 3:
        if(fvlc_unpacker.getbit_raw(vlc))
            return -7;
        return 7;
    case 2:
        fvlc_unpacker.getbits_raw(3, vlc); // can get rid of this maybe
        return -8;
    default:
        return 0;
    }
    return 0;
}

void
JPEGTDecoder::get_mv(int& vi, int& vj, BITBUFT*& vlc)
{
    vi = decode_vec(vlc);
    vj = decode_vec(vlc);
    vi = mvi.dpcm_recon(vi);
    if(vi < -8)
        vi += 16;
    else if(vi > 7)
        vi -= 16;
    vj = mvj.dpcm_recon(vj);
    if(vj < -8)
        vj += 16;
    else if(vj > 7)
        vj -= 16;
}


void
JPEGTDecoder::decode_frame(DWBYTE* ctrl, DWBYTE* overhead, BITBUFT* img,
                           BITBUFT *vlc, BITBUFT *hold_buf, BITBUFT *use_error_buf,
                           void *&app_img, int icols, int irows, int is_ref)
{
    int i, j, k, l;
    int ib, jb;
    int a, b;
    BITBUFT* control = (BITBUFT *)ctrl;
    BITBUFT* over = (BITBUFT *)overhead;
    JSAMPLE *Rlimit = Range_limit + CENTERJSAMPLE;
    MAT tpxs;
    int *tpxsp[MSZ];
    for(i = 0; i < MSZ; ++i)
        tpxsp[i] = &tpxs[i][0];
    int **tmp_pixels = &tpxsp[0];

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
    int orows = irows;
    int ocols = icols;
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
    DTracking = &this->junpacker;
    Symlist.rewind();
#endif
    mvi.init();
    mvj.init();
    if(entropy_code)
    {
//		ctrl_decoder.init(control);
        ctrl_unpacker.init(control);

        //over_decoder.init(over);
        //vlc_decoder.init(vlc);
        fvlc_unpacker.init(vlc);
        hold_decoder.init(hold_buf);
        use_error_decoder.init(use_error_buf);
        mocomp_decoder.init(over);
    }
    else
    {
        ctrl_unpacker.init(control);
    }
    dpcm_max_mag.init();
    memset(b_ue, 1, sizeof(b_ue));
    memset(b_hold, 1, sizeof(b_hold));
    start_frame();
    QTAB *qp;
    //EntropyModel& emodel = Lomo;
    //if(is_ref && !still)
    if(!still)
    {
        int u_qid;
        if(entropy_code)
            //qid = ctrl_decoder.decode_a_symbol(emodel[SYM_QID], control);
            u_qid = ctrl_unpacker.getbits_raw(5, control) & 0x1f;
        else
            u_qid = ctrl_unpacker.getbits_raw(5, control) & 0x1f;
#if 0
        printf("qidr %d\n", qid);
#endif
        if(!QidTables.find(u_qid, qp))
            return;
        qid = u_qid;
    }
    int last_dc_val = 0;
    int mocomp_dc_val = 0;
    for(i = 0, ib = 0; i < ri_rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < ri_cols; j += MSZ, ++jb)
        {
            start_block();
            //MAT *rx = &ref_xform[ib][jb];
#if 0
            int qid = *control++;
            if(qid != HOLD_BLOCK) // hold existing block
#endif
                int hold;
            if(!is_ref)
            {
                if(entropy_code)
                {
                    //hold = ctrl_decoder.decode_a_symbol(emodel[SYM_HOLD], control);
                    hold = hold_decoder.getbit(hold_buf);
                }
                else
                    hold = ctrl_unpacker.getbits_raw(1, control);
                b_hold[ib][jb] = hold;
            }
            if(is_ref || !hold)
            {
                int use_error;
                if(!is_ref || still)
                {
                    if(!still)
                    {
                        int mocomp = mocomp_decoder.getbit(over);
                        if(mocomp)
                        {
                            MAT tmp;
                            memset(tmp, 0, sizeof(tmp));
                            decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 0, tweak);
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
                        {
                            use_error = use_error_decoder.getbit(use_error_buf);
                            b_ue[ib][jb] = use_error;
                            if(use_error)
                            {
                                MAT tmp;
                                memset(tmp, 0, sizeof(tmp));
                                decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 0, tweak);
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
                                decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 0, tweak);
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
                decode_block(last_dc_val, (JCOEF *)tmp, junpacker, img, 0, tweak);

                // note: jpeg idct does all clipping and conversion in one shot
                // so it is called in "convert"
                inv_xform(tmp);
                convert(tmp, app_img, i, j, qp);
            }
            else
            {
                // block is on hold, must copy it from previous
                // frame
                int r, c;
                for(r = i; r < i + 8; ++r)
                {
                    *(dwINT32 *)&((gray **)ref_img)[r][j] = *(dwINT32 *)&last_frame[r][j] ;
                    *((dwINT32 *)&((gray **)ref_img)[r][j] + 1) = *((dwINT32 *)&last_frame[r][j] + 1);
                }
                b_ue[ib][jb] = 1;
            }
            finish_block();
        }
        finish_line();
    }
    finish_frame();
}

