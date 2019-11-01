
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/tdecode.cc 1.22 1999/01/10 16:09:39 dwight Checkpoint $
 */
#include "tcode.h"
#include "tdecode.h"
#include "qtab.h"
#include "nyb.h"
#include "packbits.h"
#include "pred.h"
#if __GNUC_MINOR__ >= 7 || defined(__linux__)
#include <netinet/in.h>
#endif
#ifdef LINUX
#include <arpa/inet.h>
#endif

#ifdef _Windows
#include <winsock.h>
#endif

TDecoder::TDecoder()
{
    //ref_xform = new XFRM;
    memset(ref_xform, 0, sizeof(ref_xform));
    ri_cols = 0;
    ri_rows = 0;
    has_error = 0;
    ref_img = 0;
    entropy_code = 0;
    still = 0;
    fis_ref = 0;
}

TDecoder::~TDecoder()
{

}

void
TDecoder::destr_fun()
{
    if(ref_img)
        dealloc_refimg();
}


void
TDecoder::error()
{
    has_error = 1;
}

void
TDecoder::reset()
{
    has_error = 0;
}

void
TDecoder::inv_xform(MAT m)
{
}

void
TDecoder::decode_from_stream(DWBYTE*& istrm, int& len, void *&app_img, int& cols, int& rows)
{
    // assume we're positioned at the beginning of a frame, and
    // go from there. if there are any errors, we set a flag
    // to indicate we should be searching for a sync token
    // in the stream (unimp.)

    // this is a bug hack-around... we know the decoder can
    // sometimes look one int past the end of the buffer
    // because of some prefetching it does, and this
    // causes infrequent crashes. since it is hard to keep
    // it from doing this, we just copy the buffer out to
    // a new area with a 4 byte end pad. this should be fixed
    // in jdhuff.cc by passing in lengths and stuff...

    DWBYTE *tmpstrm = new DWBYTE[len + 4];
    bcopy(istrm, tmpstrm, len);
    tmpstrm[len] = 0;
    tmpstrm[len + 1] = 0;
    tmpstrm[len + 2] = 0;
    tmpstrm[len + 3] = 0;
    DWBYTE *strm = tmpstrm;

    DWBYTE *ostrm = strm;

    if(len < 4)
    {
        cols = 0;
        rows = 0;
        app_img = 0;
        error();
        return;
    }
    unsigned int format = *ostrm++;
    unsigned int dummy = *ostrm++;

    unsigned short blkw = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);
    unsigned short blkh = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);

    unsigned short frm_off = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);
    unsigned short over_off = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);
    unsigned short vlc_off = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);

    unsigned short hold_off = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);
    unsigned short use_error_off = ntohs(*(unsigned short *)ostrm);
    ostrm += sizeof(unsigned short);

    // note: color and timecode info is here, but not decoding it.
    unsigned short t;
    int has_chroma;
    if(t = get_chroma_info(strm, (unsigned short *)ostrm, has_chroma))
    {
        // chroma info returns non-zero for chroma-plane
        // objects, but if there are no chroma planes to
        // decode in the stream (ie, it was produced by
        // a grayscale only codec) then has_chroma is
        // zero and we set the sampling of the chroma object
        // to 4:0:0.
        // we do this because other parts of the system
        // may be assuming that we have all the planes
        // available. note: the higher layers could negotiate
        // a grayscale decoder into existence, and we could
        // avoid all this, but it is nice to be able to
        // handle all the different types of streams without the
        // need to do fancy negotiations.
        if(!has_chroma)
        {
            set_sampling(SAMP400);
            delete [] tmpstrm;
            return;
        }
        frm_off = t;
    }

    ostrm += 2 * sizeof(unsigned short);

    int is_ref = 0;
    if(format & 0x80)
        is_ref = 1;
    fis_ref = is_ref;
    entropy_code = !!(format & 0x40);
    still = !!(format & 0x20);
    set_sampling((format & 0x10) ? SAMP411 : SAMP422);

    if(blkw > BLKC || blkh > BLKR || blkw == 0 || blkh == 0)
    {
        cols = 0;
        rows = 0;
        app_img = 0;
        error();
        return;
    }
    DWBYTE *ctrl = ostrm;
    DWBYTE *overhead = strm + over_off;
    BITBUFT *vlc = (BITBUFT *)(strm + vlc_off);

    // frame data is an offset from beginning of buffer
    BITBUFT *coded_frame = (BITBUFT *)(strm + frm_off);

    BITBUFT *hold_buf = (BITBUFT *)(strm + hold_off);
    BITBUFT *use_error_buf = (BITBUFT *)(strm + use_error_off);

    //void *img = alloc_appimg(blkw, blkh);
    void *img = 0;
    decode_frame(ctrl, overhead, coded_frame, vlc,
                 hold_buf, use_error_buf,
                 img,
                 MSZ * blkw, MSZ * blkh, is_ref);
    //app_img = img;
    app_img = ref_img;
    cols = ri_cols;
    rows = ri_rows;
    delete [] tmpstrm;
}

void
TDecoder::decode_frame(DWBYTE* ctrl, DWBYTE* overhead, BITBUFT* img,
                       BITBUFT *vlc, BITBUFT *hold_buf, BITBUFT *use_error_buf,
                       void *&app_img, int icols, int irows, int is_ref)
{
#ifdef DWYCO_CODEC
    int i, j, k, l;
    int ib, jb;
    int a, b;
    BITBUFT* control = (BITBUFT *)ctrl;
    BITBUFT* over = (BITBUFT *)overhead;

    int orows = irows;
    int ocols = icols;
    if(ref_img == 0)
    {
        ri_rows = irows & ~STRUNC;
        ri_cols = icols & ~STRUNC;
        ref_img = alloc_appimg(ri_cols / MSZ, ri_rows / MSZ);
    }
    unpacker.init(img);
    if(entropy_code)
    {
        ctrl_decoder.init(control);
        over_decoder.init(over);
        //vlc_decoder.init(vlc);
        fvlc_unpacker.init(vlc);
        hold_decoder.init(hold_buf);
        use_error_decoder.init(use_error_buf);
    }
    else
    {
        ctrl_unpacker.init(control);
    }
    dpcm_max_mag.init();
    start_frame();
    QTAB *qp;
    EntropyModel& emodel = Lomo;
    if(is_ref && !still)
    {
        int qid;
        if(entropy_code)
            qid = ctrl_decoder.decode_a_symbol(emodel[SYM_QID], control);
        else
            qid = ctrl_unpacker.getbits_raw(5, control) & 0x1f;
#if 0
        printf("qidr %d\n", qid);
#endif
        if(!QidTables.find(qid, qp))
            return;
    }
    for(i = 0, ib = 0; i < ri_rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < ri_cols; j += MSZ, ++jb)
        {
            start_block();
            MAT *rx = &ref_xform[ib][jb];
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
            }
            if(is_ref || !hold)
            {
                int use_error;
                if(!is_ref || still)
                {

                    int qid;
                    if(entropy_code)
                        qid = ctrl_decoder.decode_a_symbol(emodel[SYM_QID], control);
                    else
                        qid = ctrl_unpacker.getbits_raw(5, control) & 0x1f;
#if 0
                    printf("qid %d\n", qid);
#endif
                    if(!QidTables.find(qid, qp))
                        return;
                    if(!still)
                    {
                        if(entropy_code)
                        {
                            //use_error = ctrl_decoder.decode_a_symbol(emodel[SYM_USE_ERROR], control);
                            use_error = use_error_decoder.getbit(use_error_buf);
                        }
                        else
                            use_error = ctrl_unpacker.getbits_raw(1, control);
                    }
                }

                int meanmag;
                if(!is_ref)
                {
                    if(entropy_code)
                    {
                        meanmag = over_decoder.decode_a_symbol(emodel[SYM_NREF_DC_BITS], over);
                        meanmag = dpcm_max_mag.dpcm_recon(meanmag - 32);
                    }
                    else
                    {
                        meanmag = dpcm_max_mag.get(over);
                    }
#if 0
                    printf("dc mag %d\n", meanmag);
#endif
                }
                // get magnitude indicators
                int mag[NQUADS * NQUADS];
                for(k = 0; k < NQUADS * NQUADS; ++k)
                {
                    if(entropy_code)
                    {
                        int t = over_decoder.decode_a_symbol(emodel[is_ref ? SYM_REF_AC_BITS : SYM_NREF_AC_BITS], over);
                        mag[k] = dpcm_max_mag.dpcm_recon(t - 32);
                    }
                    else
                    {
                        mag[k] = dpcm_max_mag.get(over);
                    }
#if 0
                    printf("coef mag %d\n", mag[k]);
#endif
                }
                if(is_ref)
                {
                    memset(&(*rx)[0][0], 0, sizeof(*rx));
#if 0
                    (*rx)[0][0] = unpacker.getbits((*qp)(0, 0), meanmag, img);
#endif
                    ELTYPE dc = unpacker.getbits_raw(11, img);
                    if(dc & (1 << 10))
                        dc |= ~0 << 10;
                    (*rx)[0][0] = dc * 8;
#if 0
                    printf("refdc %d\n", dc * 8);
#endif
                }
                else
                {
                    if(use_error)
                        (*rx)[0][0] = PREDICT((*rx)[0][0]) + unpacker.getbits((*qp)(0, 0), meanmag, img);
                    else
                        (*rx)[0][0] = unpacker.getbits((*qp)(0, 0), meanmag, img);
                }

                for(k = 0; k < MSZ; ++k)
                    for(l = (k == 0); l < MSZ; ++l)
                    {
                        int quad = Baqtab[k][l];
                        int l2m = mag[quad];
                        if(l2m == NO_MAG)
                            continue;
                        int qlev = (*qp)(k, l);
#if 0
                        if(qlev == 0)
                        {
                            //(*rx)[k][l] = 0;
                            continue;
                        }
#endif
                        if(is_ref || !use_error)
                        {
                            if(qlev == 0)
                                (*rx)[k][l] = 0;
                            else
                            {
                                {
                                    int bits = qlev;
                                    if(bits > l2m)
                                        bits = l2m;
                                    ++bits;
                                    if(entropy_code /*&& bits <= 4*/)
                                    {
                                        if(fvlc_unpacker.getbit_raw(vlc) == 0)
                                            (*rx)[k][l] = 0;
                                        else
                                        {
                                            //int qval = vlc_decoder.decode_a_symbol(emodel[SYM_REF_AC(1, __bar__)], vlc);
                                            //(*rx)[k][l] = mag_recon(qval, bits, l2m);
                                            (*rx)[k][l] = unpacker.getbits(qlev, l2m, img);
                                        }
                                    }
                                    else
                                    {
                                        (*rx)[k][l] = unpacker.getbits(qlev, l2m, img);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if(qlev == 0)
                                (*rx)[k][l] = PREDICT((*rx)[k][l]);
                            else
                            {
                                int bits = qlev;
                                if(bits > l2m)
                                    bits = l2m;
                                ++bits;
                                if(entropy_code /*&& bits <= 4*/)
                                {
                                    if(fvlc_unpacker.getbit_raw(vlc) == 0)
                                        (*rx)[k][l] = PREDICT((*rx)[k][l]);
                                    else
                                    {
                                        (*rx)[k][l] = PREDICT((*rx)[k][l]) + unpacker.getbits(qlev, l2m, img);
                                    }
                                }
                                else
                                    (*rx)[k][l] = PREDICT((*rx)[k][l]) + unpacker.getbits(qlev, l2m, img);
                            }
                        }
                    }

                MAT tmp;

                bcopy(&(*rx)[0][0], &tmp[0][0], sizeof(tmp));
                //bcopy(&ref_xform[ib][jb][0][0], &Ref_xform[ib][jb][0][0], sizeof(MAT));
                // note: jpeg idct does all clipping and conversion in one shot
                // so it is called in "convert"
                inv_xform(tmp);
                convert(tmp, app_img, i, j, qp);
            }
            finish_block();
        }
        finish_line();
    }
    finish_frame();
#endif
}

void
TDecoderNP::decode_frame(DWBYTE* control, DWBYTE* over, BITBUFT* img,
                         BITBUFT *vlc, BITBUFT *hold_buf, BITBUFT *use_error_buf,
                         void *&app_img,
                         int icols, int irows, int is_ref)
{
#ifdef DWYCO_CODEC
    int i, j, k, l;
    int ib, jb;
    int a, b;

    int orows = irows;
    int ocols = icols;
    if(ref_img == 0)
    {
        ri_rows = irows & ~STRUNC;
        ri_cols = icols & ~STRUNC;
        ref_img = alloc_appimg(ri_cols / MSZ, ri_rows / MSZ);
    }
    unpacker.init(img);
    dpcm_max_mag.init();
    start_frame();
    for(i = 0, ib = 0; i < ri_rows; i += MSZ, ++ib)
    {
        start_line();
        for(j = 0, jb = 0; j < ri_cols; j += MSZ, ++jb)
        {
            start_block();
            int qid = *control++;
            if(qid != HOLD_BLOCK) // hold existing block
            {
                QTAB *qp;
                if(!QidTables.find(qid, qp))
                    return;

                MAT tmp;

                int meanmag = dpcm_max_mag.get(over);
                // get magnitude indicators
                int mag[NQUADS * NQUADS];
                for(k = 0; k < NQUADS * NQUADS; ++k)
                    mag[k] = dpcm_max_mag.get(over);
                memset(&tmp[0][0], 0, sizeof(tmp));
                tmp[0][0] = unpacker.getbits((*qp)(0, 0), meanmag, img);

                for(k = 0; k < MSZ; ++k)
                    for(l = (k == 0); l < MSZ; ++l)
                    {
                        int quad = Baqtab[k][l];
                        int l2m = mag[quad];
                        if(l2m == NO_MAG)
                            continue;
                        int qlev = (*qp)(k, l);
                        if(qlev == 0)
                            continue;
                        tmp[k][l] = unpacker.getbits(qlev, l2m, img);
                    }

                // note: jpeg idct does all clipping and conversion in one shot
                // so it is called in "convert"
                inv_xform(tmp);
                convert(tmp, app_img, i, j, qp);
            }
            finish_block();
        }
        finish_line();
    }
    finish_frame();
#endif
}


