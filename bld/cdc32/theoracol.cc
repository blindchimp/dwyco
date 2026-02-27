#include "gvchild.h"
#ifndef DWYCO_NO_THEORA_CODEC
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// this is here because on arm processors, the
// optimizer mucks up the encodersetupinfo
// fortunately, it won't impact things too badly
// without optimization here, since it is just
// glue for a lot of other things.
#pragma GCC optimize ("-O0")
#include "theoracol.h"
#include "imgmisc.h"
#include "ppmcv.h"
#include "pgmgv.h"
#include "jdcolor.h"
#include "pbmcfg.h"
#include "dwrtlog.h"
#include "sysattr.h"
#include <limits.h>
#include <stdlib.h>
#if 0 // _MSC_VER
#include "msint.h"
#else
#include <stdint.h>
#endif

// this video codec presents some new problems for the
// dvcc world: in the past, the codec contained no rate control.
// all of the rate control was handled by the networking layer
// by allowing it to signal the entire chain of capture->coding
// that it would be ok to process a new frame (i.e. there is enough
// b/w available to handle a new frame.)
// in this setup, the only rate control was really frame dropping
// which works fine for video conferencing.
// since theora contains a built-in rate control system,
// it causes a bit of a clash. i'm not sure if it would be better to
// somehow reflect the networks rate control parameters into the codec
// or just configure the codec in some way to allow it to produce
// a constant quality bit stream and feed it as if it was an old-style codec.
// part of the reason theora does this is because i think it is expecting
// a constant stream of frames at a fixed frame rate, which may not
// be realistic in this environment.

// this is the common state information that is used to recreate
// the decoder state in many decoders, given one coder
// this is the ogg stream we create on first codec creation,
// and then subsequently replay into the decoders as they
// are created. with any luck, we can just wait for a key frame to
// come by, and start the decoding process there.
// first index is "quality" level
// second index is dimension

#define TH_SQCIF 0  // 128x96
#define TH_QCIF 1   // 176x144
#define TH_CIF 2    // 352x288
#define TH_4CIF 3    // 704x576
// these are closer to common formats provided by webcams
#define TH_80x64 4
#define TH_160x128 5
#define TH_320x240 6
#define TH_640x480 7
#define TH_N_ENCODER_SIZES 15
#define TH_N_ENCODER_QUALITIES 5
// don't rearrage anything in this array, as the index
// is put into the video stream, and if you change it
// around, the receiver will get confused.
static int Sizes[TH_N_ENCODER_SIZES][2] =
{
    128, 96,
    176, 144,
    352, 288,
    704, 576,
    80, 64,
    160, 128,
    320, 240,
    640, 480,
    720, 480,
    // some HD sizes for 16:9 and 4:3
    1280, 720,
    1280, 960,
    1600, 1200,
    1920, 1088,
    1920, 1200, // not hd size,  wuxga
    2048, 1536

};

static DWBYTE *EncoderSetupInfo[TH_N_ENCODER_QUALITIES][TH_N_ENCODER_SIZES];
static int EncoderSetupLen[TH_N_ENCODER_QUALITIES][TH_N_ENCODER_SIZES];

static int
match_cols_rows(int cols, int rows, int& szidx_out)
{
    for(int i = 0; i < TH_N_ENCODER_SIZES; ++i)
    {
        if(cols == Sizes[i][0] && rows == Sizes[i][1])
        {
            szidx_out = i;
            return 1;
        }
    }
    return 0;
}

// return next bigger size for a frame of size cols x rows
// or the exact frame size
static int
find_next_bigger_size(int cols, int rows, int& szidx_out)
{
    int mini = -1;
    int mindiff = INT_MAX;
    for(int i = 0; i < TH_N_ENCODER_SIZES; ++i)
    {
        if(cols == Sizes[i][0] && rows == Sizes[i][1])
        {
            szidx_out = i;
            return 1;
        }
        if(cols > Sizes[i][0] || rows > Sizes[i][1])
            continue;
        int diff = (Sizes[i][0] - cols) + (Sizes[i][1] - rows);
        if(diff < mindiff)
        {
            mindiff = diff;
            mini = i;
        }
    }
    if(mini == -1)
        return 0;
    szidx_out = mini;
    return 1;
}

// find the size that results in the smallest change in
// area to the given cols and rows. might return a smaller
// dimension as well, rather than just padding.
static int
find_closest_size(int cols, int rows, int& szidx_out)
{
    int area_diff[TH_N_ENCODER_SIZES];
    int min_area_diff = INT_MAX;
    for(int i = 0; i < TH_N_ENCODER_SIZES; ++i)
    {
        area_diff[i] = abs(cols - Sizes[i][0]) + abs(rows - Sizes[i][1]);
        if(area_diff[i] == 0)
        {
            szidx_out = i;
            return 1;
        }
        if(area_diff[i] < min_area_diff)
        {
            min_area_diff = area_diff[i];
            szidx_out = i;
        }
    }
    return 0;
}

/*
typedef struct {
  unsigned char *packet;
  long  bytes;
  long  b_o_s;
  long  e_o_s;

  ogg_int64_t  granulepos;

  ogg_int64_t  packetno;     /* sequence number for decode; the framing
				knows where there's a hole in the data,
				but we need coupling so that the codec
				(which is in a seperate abstraction
				layer) also knows about the gap
} ogg_packet;
*/

// NOTE: have to be fixed on big-ending machines
static int
op_out(ogg_packet *op, DwString& out)
{
    DwString c;
    c += DwString((const char *)&op->b_o_s, 0, 4);
    c += DwString((const char *)&op->e_o_s, 0, 4);
    c += DwString((const char *)&op->granulepos, 0, 8);
    c += DwString((const char *)&op->packetno, 0, 8);
    c += DwString((const char *)op->packet, 0, op->bytes);
    int len = c.length();
    out += DwString((const char *)&len, 0, 4);
    out += c;
    return 1;
}

// need a limit on this to avoid potential overflows
static int
op_in(DWBYTE *&inbuf, ogg_packet *op)
#if !defined(_Windows) && defined(__clang__)
__attribute__ ((optnone))
#endif
{
#if 0
    int total_len = *((int *)inbuf);
    inbuf += 4;
    op->b_o_s = *((int *)inbuf);
    inbuf += 4;
    op->e_o_s = *(int *)inbuf;
    inbuf += 4;
#endif
    int total_len;
    memcpy(&total_len, inbuf, 4);
    memcpy(&op->b_o_s, inbuf + 4, 4);
    memcpy(&op->e_o_s, inbuf + 8, 4);
    inbuf += 12;
// hack for some arm processors where there is an alignment
// restriction for 8 byte items (and this will always end up on
// a 4 byte boundary.)
    {
        //unsigned int a1 = *(unsigned int *)inbuf;
        //unsigned int a2 = *(unsigned int *)(inbuf + 4);
        unsigned int a1;
        unsigned int a2;
        memcpy(&a1, inbuf, 4);
        memcpy(&a2, inbuf + 4, 4);
        op->granulepos = ((long long)a2 << 32) | a1;
    }
#if 0
    op->granulepos = *(ogg_int64_t *)inbuf;
#endif
    inbuf += 8;
    {
        //unsigned int a1 = *(unsigned int *)inbuf;
        //unsigned int a2 = *(unsigned int *)(inbuf + 4);
        unsigned int a1;
        unsigned int a2;
        memcpy(&a1, inbuf, 4);
        memcpy(&a2, inbuf + 4, 4);
        op->packetno = ((long long)a2 << 32) | a1;
    }
#if 0
    op->packetno = *(ogg_int64_t *)inbuf;
#endif
    inbuf += 8;
    op->bytes = total_len - 24;
    op->packet = (unsigned char *)inbuf;
    inbuf += op->bytes;
    return 1;
}

static DwString
gen_encoder_info(th_info& cur_info, th_enc_ctx*& cur_coder_ctx, int q, int w, int h)
{
    DwString working_buf;
    th_info_init(&cur_info);
    cur_info.frame_width = w;
    cur_info.frame_height = h;
    cur_info.pic_width = cur_info.frame_width;
    cur_info.pic_height = cur_info.frame_height;
    cur_info.pic_x = 0;
    cur_info.pic_y = 0;
    // we don't really have a color space this precise, since
    // we are usually capturing from cheap webcams. but we'll
    // set it here anyways for completeness.
    cur_info.colorspace = TH_CS_ITU_REC_470M;
    // like our current codecs, all the rigamarole below just
    // converts the inputs from the "sampling" which comes straight
    // from the driver (more or less) into 420. you *might* be able to
    // match up the driver format with one of theora's formats and
    // save some conversion time, but that may be a lose, depending on
    // what theora does internally. for this app, it isn't that impportant
    // because the frame sizes are small, and the quality is usually pretty
    // low as well.
    cur_info.pixel_fmt = TH_PF_420;
    // need to probably calculate this from the policy parameters, ok
    // to fix it for now. from docs, if bitrate==0, quality based vbr is
    // enabled.
    cur_info.target_bitrate = sysattr_get_int("us-video-coder-theora-quality-cbr", 0);
    // this is 0 to 63, but who knows, maybe we should just wire it to
    // a single value.
    int u = sysattr_get_int("us-video-coder-theora-quality-upper", 63);
    int l = sysattr_get_int("us-video-coder-theora-quality-lower", 0);
    int s = sysattr_get_int("us-video-coder-theora-quality-step", 5);
    if(s == 0)
        s = 1;

    cur_info.quality = ((u - l) / s) * q + l ;
    if(cur_info.quality < 0)
        cur_info.quality = 0;
    if(cur_info.quality > 63)
        cur_info.quality = 63;
    cur_info.fps_numerator = 12;
    cur_info.fps_denominator = 1;
    // note: use default of 6 for now, need to look into this
    // wrt reference frame generation.
    //cur_info.keyframe_granule_shift = xx;
    cur_coder_ctx = th_encode_alloc(&cur_info);
    if(cur_coder_ctx == 0)
        oopanic("bad theora encoder");
    {
        // set max speed, which from the source mainly seems to
        // disable motion comp, which is ok for video conferencing
        // situations
        int max_sp;
        if(th_encode_ctl(cur_coder_ctx, TH_ENCCTL_GET_SPLEVEL_MAX, &max_sp, sizeof(max_sp)) == 0)
        {
	    --max_sp;
            th_encode_ctl(cur_coder_ctx, TH_ENCCTL_SET_SPLEVEL, &max_sp, sizeof(max_sp));
        }
    }
    // we have to flush this stuff out before doing any
    // video coding, so just dump it into the workbuf
    // where final_package will find it later.
    th_comment tc;
    th_comment_init(&tc);
    th_comment_add(&tc, (char *)"dwyco");
    ogg_packet op;
    while(th_encode_flushheader(cur_coder_ctx, &tc, &op))
    {
        op_out(&op, working_buf);
    }
    th_comment_clear(&tc);
    return working_buf;
}

static
DWBYTE *
gen_encoder_setup(int qual_idx, int size_idx, int& len_out)
{
    th_info cur_info;
    th_enc_ctx *ec;

    DwString tmp = gen_encoder_info(cur_info, ec, qual_idx, Sizes[size_idx][0], Sizes[size_idx][1]);
    DWBYTE *ret = new DWBYTE[tmp.length()];
    memcpy(ret, tmp.c_str(), tmp.length());
    len_out = tmp.length();
    th_encode_free(ec);
    return ret;
}

#if 0
static
void
setup_encoder_headers()
{
    for(int q = 0; q < TH_N_ENCODER_QUALITIES; ++q)
    {
        for(int s = 0; s < TH_N_ENCODER_SIZES; ++s)
        {
            th_info cur_info;
            th_enc_ctx *ec;

            DwString tmp = gen_encoder_info(cur_info, ec, q, Sizes[s][0], Sizes[s][1]);
            EncoderSetupInfo[q][s] = new DWBYTE[tmp.length()];
            memcpy(EncoderSetupInfo[q][s], tmp.c_str(), tmp.length());
            EncoderSetupLen[q][s] = tmp.length();
            th_encode_free(ec);
        }
    }
}
#endif

// unlike our other codecs, the theora stuff doesn't process
// luma and chroma planes separately. so we just do all the work
// right here in the codercolor object instead of dispatching out
// to the individual codec objects.
CDCTheoraCoderColor::CDCTheoraCoderColor()
{

//    if(EncoderSetupInfo[0][0] == 0)
//    {
//        setup_encoder_headers();
//    }
    cur_coder_ctx = 0;
    cur_size = -1;
    cur_qual = -1;
    cur_rows = -1;
    cur_cols = -1;
}

CDCTheoraCoderColor::~CDCTheoraCoderColor()
{
    if(cur_coder_ctx)
        th_encode_free(cur_coder_ctx);
    cur_coder_ctx = 0;
    th_info_clear(&cur_info);
}

int
CDCTheoraCoderColor::get_quality()
{
    return 4;
    // with the other codecs, a file on the disk
    // gives the mapping between qualities and
    // name, but here we just hack this up to use
    // the existing stuff
    if(strcmp(policy, "better") == 0)
        return 2;
    if(strcmp(policy, "better-yet") == 0)
        return 3;
    if(strcmp(policy, "best") == 0)
        return 4;
    else
        return 1;
}

DWBYTE *
CDCTheoraCoderColor::code_preprocess(gray **img_l, gray **img_cb, gray **img_cr,
                                     int cols, int rows, int is_ref, int& len_out, void *captured_ppm)
{
    if(!busy())
        set_busy(1);
    else
        oopanic("color code busy");
    // shortcut for when we are just previewing
    if(captured_ppm && inhibit_coding)
    {
        display_img_2b_coded((pixel **)captured_ppm, cols, rows);
        set_busy(0);
        return 0;
    }
    // note: user defined up-front cropping and subsampling is disabled for
    // this codec right now. no big deal, since it isn't used by any clients
    // at the moment anyways.
    do_crop = 0;
    do_subsample = 0;
    do_pad = 1;

    if(do_crop == 0)
    {
        x = 0;
        y = 0;
        h = rows;
        w = cols;
    }

    // check to see if we need to reinitialize the codec
    // need to check sampling too, maybe later.
    // the reference frame generation for theora is also
    // a bit different. will have to figure out if i can
    // force generation of reference frames on the fly.

    if(cur_coder_ctx == 0 || cur_cols != cols || cur_rows != rows)
    {
        int szidx;
        if(!find_next_bigger_size(cols, rows, szidx))
        {
            // don't crash, stupid windows cams may become
            // unusable cuz they store the last size in the drivers
            set_busy(0);
            return 0;
        }
        if(szidx != cur_size || get_quality() != cur_qual)
        {
        }
        // note: leak, have to clear out old coder info
        cur_size = szidx;
        cur_qual = get_quality();
        cur_cols = cols;
        cur_rows = rows;
        gen_encoder_info(cur_info, cur_coder_ctx, cur_qual, Sizes[szidx][0], Sizes[szidx][1]);
        cur_info.pic_height = rows;
        cur_info.pic_width = cols;

    }
    if(is_ref)
    {
        ogg_uint32_t zero = 0;
        th_encode_ctl(cur_coder_ctx, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &zero, sizeof(zero));
    }
    else
    {
        ogg_uint32_t six = 6;
        th_encode_ctl(cur_coder_ctx, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &six, sizeof(six));
    }
#if 0
    int rcols = (cols & 15) ? ((cols + 16) & ~15) : cols;
    int rrows = (rows & 15) ? ((rows + 16) & ~15) : rows;
    if(rcols != cur_info.frame_width || rrows != cur_info.frame_height)
    {
        oopanic("theora will choke");
    }
#endif

    if((sampling == SAMPYUV9 || sampling == SAMPYUV12) && img_cb && img_cr)
        codeyuv(img_l, img_cb, img_cr, cols, rows, is_ref, len_out,
                sampling == SAMPYUV9 ? 4 : 2);
    else
    {
        // one of the RGB formats, 3 full sized y cr cb planes, captured ppm
        // is full RGB ppm
        gray **nimg[3];
        gray **nimg2[3];
        int i;
        int r[3], c[3], o_r[3], o_c[3];
        int nplanes = 3;

        if(sampling == SAMP400 || img_cb == 0 || img_cr == 0)
        {
            nplanes = 1;
            sampling = SAMP400;
        }

        int ocols = cols;
        int orows = rows;

        gray **a[3];
        a[0] = img_l;
        a[1] = img_cb;
        a[2] = img_cr;
#if 0
        static int cnt;
        outpgm("il", cnt, img_l, cols, rows);
        outpgm("ib", cnt, img_cb, cols, rows);
        outpgm("ir", cnt, img_cr, cols, rows);
#endif
        // perform up-front croping and subsampling on
        // all three planes identically.
        //
        for(i = 0; i < nplanes; ++i)
        {
            r[i] = orows;
            c[i] = ocols;
            o_r[i] = orows;
            o_c[i] = ocols;
            if(do_subsample)
            {
                nimg[i] = subsample2_1(a[i], &r[i], &c[i]);
                pgm_freearray(a[i], orows);
                o_r[i] = r[i];
                o_c[i] = c[i];
                if(do_crop == 0)
                {
                    w = c[i];
                    h = r[i];
                }
                if(i == 0) // use luma plane as guide
                    captured_ppm = 0;
            }
            else
                nimg[i] = a[i];
//do_pad = 1;
            //if((nimg2[i] = crop(nimg[i], &c[i], &r[i], x, y, w, h, do_pad, 1)) != 0)
            if((nimg2[i] = cond_pad(nimg[i], c[i], r[i], cur_info.frame_width, cur_info.frame_height)) != 0)
            {
                pgm_freearray(nimg[i], o_r[i]);
                o_r[i] = r[i];
                o_c[i] = c[i];
                r[i] = cur_info.frame_height;
                c[i] = cur_info.frame_width;
                if(i == 0)
                    captured_ppm = 0;
            }
            else
                nimg2[i] = nimg[i];
        }
#if 0
        outpgm("il2", cnt, nimg2[0], c[0], r[0]);
        outpgm("ib2", cnt, nimg2[1], c[1], r[1]);
        outpgm("ir2", cnt, nimg2[2], c[2], r[2]);
#endif
        gray **chr[3];
        gray **chr2[3];

        // now subsample chroma planes depending on the
        // selected 422 or 411 sampling
        for(i = 1; i < nplanes; ++i)
        {
            chr[i] = subsample2_1(nimg2[i], &r[i], &c[i]);
            // note: don't free it here, may need it for display later.
            o_r[i] = r[i];
            o_c[i] = c[i];
#if 0
            if(sampling == SAMP411)
            {
                chr2[i] = subsample2_1(chr[i], &r[i], &c[i]);
                pgm_freearray(chr[i], or[i]);
                chr[i] = chr2[i];
                or[i] = r[i];
                oc[i] = c[i];
            }
#endif

            // note: always pad, since we don't want a little part
            // of the picture to be grayscale
            if((chr2[i] = crop(chr[i], &c[i], &r[i], 0, 0, c[i], r[i], 1, 0)) != 0)
            {
                pgm_freearray(chr[i], o_r[i]);
            }
            else
                chr2[i] = chr[i];
        }
#if 0
        outpgm("ib3", cnt, chr2[1], c[1], r[1]);
        outpgm("ir3", cnt, chr2[2], c[2], r[2]);
        ++cnt;
#endif

        if(sampling == SAMP400)
            display_img_2b_coded(nimg2[0], c[0], r[0]);
        else if(captured_ppm)
            display_img_2b_coded((pixel **)captured_ppm, cols, rows);
        else
        {
            // note: don't want to use c[0] since that is padded out to	frame size
            // for theora, want to display the image in orig size. we can do this
            // because we know we padded out and the image is upper-left justified in the
            // ppm.
            display_img_2b_coded(nimg2[0], nimg2[1], nimg2[2], cols, rows /*c[0], r[0]*/);
        }
        for(i = 1; i < nplanes; ++i)
            pgm_freearray(nimg2[i], r[0]); // note: r[0] because we want to free orig image size

        if(!inhibit_coding)
        {
#ifndef CDC32
            // automatically toggle for icuii
            if(c[0] < 320 && r[0] < 240)
                set_policy("best");
            else if(c[0] < 640 && r[0] < 480)
                set_policy("better-yet");
            else
                set_policy("better");
#endif
            // here is where we set up for theora specifically
            th_ycbcr_buffer tb;
            tb[0].width = cur_info.frame_width; //c[0];
            tb[0].height = cur_info.frame_height; //r[0];
            // note: bad, assuming both are pointers to chars, and
            // that the pgm buffers are contiguous (tho this doesn't
            // have to be the case, since pgm stores pointers to each
            // row.) if you set up pgm to not use contiguous buffers for
            // some odd reason, this will lose (you will have to perform
            // a buffer conversion to store all the data in contiguous rows.)
            // also assumes there is at least 1 row
            tb[0].stride = (char *)&nimg2[0][1][0] - (char *)&nimg2[0][0][0];
            tb[0].data = &nimg2[0][0][0];

            tb[1].width = c[1];
            tb[1].height = r[1];
            tb[1].stride = (char *)&chr2[1][1][0] - (char *)&chr2[1][0][0];
            tb[1].data = &chr2[1][0][0];

            tb[2].width = c[2];
            tb[2].height = r[2];
            tb[2].stride = (char *)&chr2[2][1][0] - (char *)&chr2[2][0][0];
            tb[2].data = &chr2[2][0][0];

            if(th_encode_ycbcr_in(cur_coder_ctx, tb) != 0)
            {
                GRTLOG("bad theora encode", 0, 0);
                set_busy(0);
                return 0;
            }
            ogg_packet op;
            while(th_encode_packetout(cur_coder_ctx, 0, &op) != 0)
            {
                op_out(&op, working_buf);
            }

#if 0
            luma->code_frame(nimg2[0], c[0], r[0], is_ref, policy);
            if(sampling != SAMP400)
            {
                cb->code_frame(chr2[1], c[1], r[1], is_ref, policy);
                cr->code_frame(chr2[2], c[2], r[2], is_ref, policy);
            }
#endif
        }
        else
        {

        }
        // note: theora makes a copy of the frames, unlike our codecs
        // that just take possession of them.
        pgm_freearray(nimg2[0], r[0]);
        if(sampling != SAMP400)
        {
            pgm_freearray(chr2[1], r[1]);
            pgm_freearray(chr2[2], r[2]);
        }
    }

    DWBYTE *b;
    if(!inhibit_coding)
    {
        b = final_package(len_out);
    }
    else
    {
        b = 0;
        len_out = 0;
    }
    set_busy(0);
    return b;
}

// don't do any user defined subsampling, etc.
// just code and send it.
//
void
CDCTheoraCoderColor::codeyuv(gray **img_l, gray **img_cb, gray **img_cr,
                             int cols, int rows, int is_ref, int& len_out, int subsamp)
{
    //cr->sampling = (subsamp == 4 ? SAMP411 : SAMP422);
    //cb->sampling = (subsamp == 4 ? SAMP411 : SAMP422);

    gray **a[2];
    int r[2], c[2];
    int i;

    {
        gray **nimg[3];
        gray **nimg2[3];
        int i;
        int ri[3], ci[3], o_r[3], o_c[3];
        int nplanes = 3;

        int ocols = cols;
        int orows = rows;

        gray **a[3];
        a[0] = img_l;
        a[1] = img_cb;
        a[2] = img_cr;
#if 0
        static int cnt;
        outpgm("il", cnt, img_l, cols, rows);
        outpgm("ib", cnt, img_cb, cols, rows);
        outpgm("ir", cnt, img_cr, cols, rows);
#endif
        // perform up-front cropping and subsampling on
        // all three planes identically.
        //
        for(i = 0; i < nplanes; ++i)
        {
            if(i == 0)
            {
                ri[i] = orows;
                ci[i] = ocols;
                o_r[i] = orows;
                o_c[i] = ocols;
            }
            else
            {
                // chroma planes are subsampled already
                ri[i] = orows / subsamp;
                ci[i] = ocols / subsamp;
                o_r[i] = orows / subsamp;
                o_c[i] = ocols / subsamp;
            }
            if(do_subsample)
            {
                nimg[i] = subsample2_1(a[i], &ri[i], &ci[i]);
                pgm_freearray(a[i], orows);
                o_r[i] = ri[i];
                o_c[i] = ci[i];
                if(do_crop == 0)
                {
                    w = ci[i];
                    h = ri[i];
                }
            }
            else
                nimg[i] = a[i];
#if 0
            if((nimg2[i] = crop(nimg[i], &ci[i], &ri[i], x, y, w, h, do_pad, 1)) != 0)
            {
                pgm_freearray(nimg[i], or[i]);
                or[i] = ri[i];
                oc[i] = ci[i];
            }
            else
#endif
                nimg2[i] = nimg[i];
        }
        img_l = nimg2[0];
        img_cb = nimg2[1];
        img_cr = nimg2[2];
        cols = ci[0];
        rows = ri[0];
        r[0] = ri[1];
        r[1] = ri[2];
        c[0] = ci[1];
        c[1] = ci[2];
    }
    // upsample the chroma so we can display it.
    // we wouldn't need to do this if the hardware preview
    // was adequate (sometimes we actually need to see what is
    // being captured.)

    a[0] = img_cb;
    a[1] = img_cr;
    int o_r[2];
    int o_c[2];
    for(i = 0; i < 2; ++i)
    {
        //r[i] = rows / subsamp;
        //c[i] = cols / subsamp;
        o_r[i] = r[i];
        o_c[i] = c[i];
        gray **b = (subsamp == 4 ? upsample2_2 : upsample1_2 )(a[i], &c[i], &r[i]);
        a[i] = b;
    }

    display_img_2b_coded(img_l, a[0], a[1], cols, rows);
    for(i = 0; i < 2; ++i)
        pgm_freearray(a[i], r[i]);

    if(!inhibit_coding)
    {
        // theora wants the luma to be a multiple of 16, and the
        // chroma to be at least half the size of the luma (ie, no
        // YUV9-style input format is supported.
        a[0] = img_cb;
        a[1] = img_cr;
        if(subsamp == 4)
        {
            // upsample the chroma to be at least yuv12
            for(i = 0; i < 2; ++i)
            {
                r[i] = o_r[i];
                c[i] = o_c[i];
                gray **b = upsample1_2(a[i], &c[i], &r[i]);
                pgm_freearray(a[i], o_r[i]);
                a[i] = b;
                o_r[i] = r[i];
                o_c[i] = c[i];
            }
        }
        // pad out frames
        // note: always pad, since we don't want a little part
        // of the picture to be grayscale

        // here we have a luma plane of cols x rows big
        // and two chroma planes of c[] x r[] big

        // now pad it out to the size of the frame we told theora to use


        gray **img_l2;
        int rl = rows;
        int cl = cols;
        int img_lr = -1;
        int img_lc = -1;
        if((img_l2 = cond_pad(img_l, cols, rows, cur_info.frame_width, cur_info.frame_height)) != 0)
        {
            pgm_freearray(img_l, rows);
            img_l = img_l2;
            img_lr = cur_info.frame_height;
            img_lc = cur_info.frame_width;
        }
        else
        {
            img_lr = rows;
            img_lc = cols;
        }

        gray **chr2[2];
        for(i = 0; i < 2; ++i)
        {
            //r[i] = rows / subsamp;
            //c[i] = cols / subsamp;
            r[i] = o_r[i];
            c[i] = o_c[i];
            if((chr2[i] = cond_pad(a[i], c[i], r[i], cur_info.frame_width / 2, cur_info.frame_height / 2)) != 0)
            {
                pgm_freearray(a[i], o_r[i]);
                r[i] = cur_info.frame_height / 2;
                c[i] = cur_info.frame_width / 2;
            }
            else
                chr2[i] = a[i];
        }
#if 0
        // note: set "to_be_subsampled" to 1 because we know it will pad to
        // 16 instead of 8 (which is what theora wants). not good, but ok for now.
        if((img_l2 = crop(img_l, &cl, &rl, 0, 0, cols, rows, 1, 1)) != 0)
        {
            pgm_freearray(img_l, rows);
            img_l = img_l2;
            rows = rl;
            cols = cl;
        }

        //a[0] = img_cb;
        //a[1] = img_cr;
        gray **chr2[2];
        for(i = 0; i < 2; ++i)
        {
            //r[i] = rows / subsamp;
            //c[i] = cols / subsamp;
            r[i] = o_r[i];
            c[i] = o_c[i];
            if((chr2[i] = crop(a[i], &c[i], &r[i], 0, 0, c[i], r[i], 1, 0)) != 0)
            {
                pgm_freearray(a[i], o_r[i]);
            }
            else
                chr2[i] = a[i];
        }
#endif

#ifndef CDC32
        // automatically toggle for icuii
        if(cl < 320 && rl < 240)
            set_policy("best");
        else if(cl < 640 && rl < 480)
            set_policy("better-yet");
        else
            set_policy("better");
#endif
//printf("luma\n");
        // here is where we set up for theora specifically
        th_ycbcr_buffer tb;
        tb[0].width = img_lc;
        tb[0].height = img_lr;
        // note: bad, assuming both are pointers to chars, and
        // that the pgm buffers are contiguous (tho this doesn't
        // have to be the case, since pgm stores pointers to each
        // row.) if you set up pgm to not use contiguous buffers for
        // some odd reason, this will lose (you will have to perform
        // a buffer conversion to store all the data in contigous rows.)
        // also assumes there is at least 1 row
        tb[0].stride = (char *)&img_l[1][0] - (char *)&img_l[0][0];
        tb[0].data = &img_l[0][0];

        tb[1].width = c[0];
        tb[1].height = r[0];
        tb[1].stride = (char *)&chr2[0][1][0] - (char *)&chr2[0][0][0];
        tb[1].data = &chr2[0][0][0];

        tb[2].width = c[1];
        tb[2].height = r[1];
        tb[2].stride = (char *)&chr2[1][1][0] - (char *)&chr2[1][0][0];
        tb[2].data = &chr2[1][0][0];

        if(th_encode_ycbcr_in(cur_coder_ctx, tb) != 0)
        {
            GRTLOG("bad theora encode", 0, 0);
            return;
        }
        pgm_freearray(img_l, rl);
        pgm_freearray(chr2[0], r[0]);
        pgm_freearray(chr2[1], r[1]);

        ogg_packet op;
        while(th_encode_packetout(cur_coder_ctx, 0, &op) != 0)
        {
            op_out(&op, working_buf);
        }
#if 0
        luma->code_frame(img_l, cl, rl, is_ref, policy);
        cb->code_frame(chr2[0], c[0], r[0], is_ref, policy);
        cr->code_frame(chr2[1], c[1], r[1], is_ref, policy);
#endif
    }
    else
    {
        pgm_freearray(img_l, rows);
        pgm_freearray(img_cb, rows / subsamp);
        pgm_freearray(img_cr, rows / subsamp);
    }
}

DWBYTE *
CDCTheoraCoderColor::final_package(int& len)
{
// need to encode the quality and dimensions (or at least the index
    // in the table) for the decoder
    DWBYTE *b = new DWBYTE[working_buf.length() + 6];
    b[0] = ((cur_size << 4) & 0xf0) | (cur_qual & 0xf);
    b[1] = 0; // reserved
    // need the actual picture size here, so we can crop to the
    // right size on decode
    ((int16_t *)b)[1] = cur_cols;
    ((int16_t *)b)[2] = cur_rows;
    memcpy(b + 6, working_buf.c_str(), working_buf.length());
    len = working_buf.length() + 6;
    working_buf = "";
    return b;
}


void
CDCTheoraCoderColor::display_img_2b_coded(gray **y, gray **cb, gray **cr, int cols, int rows)
{
    DwVec<int> tmp = gvs;
    tmp.append(gv_id);
    int n = tmp.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int gg = tmp[i];
        if(gg <= 0)
            continue;
        PPMIMG p = ppm_allocarray(cols, rows);
        JSAMPIMAGE a = new JSAMPARRAY[3];
        a[0] = y;
        a[1] = cb;
        a[2] = cr;
        ycc_rgb_convert(a, 0, (unsigned char **)p, cols, rows);
        ppm_to_colorview(p, cols, rows, gg);
        ppm_freearray(p, rows);
        delete [] a;
    }
}

void
CDCTheoraCoderColor::display_img_2b_coded(gray **g, int cols, int rows)
{
    DwVec<int> tmp = gvs;
    tmp.append(gv_id);
    int n = tmp.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int gg = tmp[i];
        pgm_to_grayview(g, cols, rows, gg);
    }
}

void
CDCTheoraCoderColor::display_img_2b_coded(pixel **g, int cols, int rows)
{
    DwVec<int> tmp = gvs;
    tmp.append(gv_id);
    int n = tmp.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int gg = tmp[i];
        if(gg <= 0)
            continue;
        ppm_to_colorview(g, cols, rows, gg);
    }
}

static void
display_info(const char *str, int gv_id)
{
    set_caption_by_id(gv_id, str);
}

void
CDCTheoraCoderColor::display_info(const char *str)
{
    ::display_info(str, gv_id);
}

CDCTheoraDecoderColor::CDCTheoraDecoderColor()
{
    setup_info = 0;
    cur_dec_ctx = 0;

    // ok, we need to just generate a few common formats and use those
    // decoder contexts. i think theora may pose some problems for our
    // anything-goes streaming stuff.
    // also note, since the stream from the coder may come in
    // in any state, we need to wait until we get a key frame
    // in order to start decoding, which means we might have to
    // change the upper levels to understand that some of the
    // data is being dropped.

    // note: we may be playing back something, so we need the encoder
    // info even if we don't have an encoder object laying around.
//    if(EncoderSetupInfo[0][0] == 0)
//    {
//        setup_encoder_headers();
//    }
    cur_fqual = -1;
    cur_fsize = -1;
    cur_fcols = -1;
    cur_frows = -1;
    seeking_keyframe = 0;

}

CDCTheoraDecoderColor::~CDCTheoraDecoderColor()
{
    if(cur_dec_ctx)
    {
        th_decode_free(cur_dec_ctx);
        cur_dec_ctx = 0;
    }
}

void
CDCTheoraDecoderColor::setup_decoder_state(int fsize, int fqual)
{
    // note: can't do this here anymore. have to wait for first packet that has the
    // dimensions and quality in it.
    ogg_packet op;
    th_comment tc;
    if(EncoderSetupInfo[fqual][fsize] == 0)
    {
        int len;

        EncoderSetupInfo[fqual][fsize] = gen_encoder_setup(fqual, fsize, len);
        EncoderSetupLen[fqual][fsize] = len;
    }
    DWBYTE *obuf = EncoderSetupInfo[fqual][fsize];
    int len = EncoderSetupLen[fqual][fsize];
    DWBYTE *buf = obuf;
    DWBYTE *ebuf = buf + len;
    // ok, according to the spec, there will be 3,
    // so we'll go with that for now
    th_info_init(&cur_info);
    th_comment_init(&tc);
    setup_info = 0;
    for(int i = 0; i < 3; ++i)
    {
        op_in(buf, &op);
        int ret = th_decode_headerin(&cur_info, &tc, &setup_info, &op);
        if(ret < 0)
        {
            GRTLOG("bogus theora header", 0, 0);
            return;
        }
    }
    if(buf != ebuf)
    {
        GRTLOG("bogus static headers", 0, 0);
        return;
    }
    cur_dec_ctx = th_decode_alloc(&cur_info, setup_info);
    th_setup_free(setup_info);
    th_comment_clear(&tc);
    setup_info = 0;
    seeking_keyframe = 1;
    cur_fqual = fqual;
    cur_fsize = fsize;
}

// note: may need to check if buf and len have to reflect how much was
// consumed from the stream, think it does
void
CDCTheoraDecoderColor::decode_from_stream(DWBYTE*& buf, int& len, void *&vimg, int &cols, int& rows)
{
    ogg_packet op;
    th_comment tc;
    if(len == 0)
        return;
    vimg = 0;

    int fsize = ((*buf) >> 4) & 0xf;
    int fqual = ((*buf) & 0xf);
    int fcols = ((int16_t *)buf)[1];
    int frows = ((int16_t *)buf)[2];
    if(fsize < 0 || fsize >= TH_N_ENCODER_SIZES ||
            fqual < 0 || fqual >= TH_N_ENCODER_QUALITIES)
    {
        GRTLOG("decoding theora: size/quality out of range", 0, 0);
        vimg = 0;
        cols = 0;
        rows = 0;
        return;
    }
    if(cur_fsize != fsize || cur_fqual != fqual)
    {
        // recompute decoder state.
        // note leak, have to clear out old decoder state.
        setup_decoder_state(fsize, fqual);
    }
    buf += 6;
    len -= 6;
    // the docs are unclear whether this is necessary or not
    DWBYTE *ebuf = buf + len;
    op_in(buf, &op);
    if(buf > ebuf)
    {
        GRTLOG("buffer overflow decoding theora", 0, 0);
        return;

    }
    if(th_packet_isheader(&op))
    {
        GRTLOG("headers in theora stream", 0, 0);
        return;
    }
    else if(seeking_keyframe)
    {
        if(!th_packet_iskeyframe(&op))
        {
            vimg = 0;
            cols = 0;
            rows = 0;
            return;
        }
        else
        {
            seeking_keyframe = 0;
        }

    }

    int ret = th_decode_packetin(cur_dec_ctx, &op, 0);
    // might be something we can do about this dupframe stuff (dropped
    // frames, but for now, we don't bother, and just redo the work.
    if(ret != 0)
    {
        if(ret != TH_DUPFRAME)
        {
            GRTLOG("bogus theora decode2", 0, 0);
            return;
        }
    }
    th_ycbcr_buffer tb;
    th_decode_ycbcr_out(cur_dec_ctx, tb);

    // create a ppm for display
    // we set the codec to do pf420, so we just do that for now.
    if(cur_info.pixel_fmt != TH_PF_420)
    {
        GRTLOG("wrong pixel format in decode", 0, 0);
        return;
    }

    // this is a little different from other codecs in the system.
    // since we get a whole frame, and not 3 components independently
    // we just create a ppm vimg (instead of a pgm, which is merged externally later.)

    gray **g[3];
    for(int p = 0; p < 3; ++p)
    {
        g[p] = pgm_allocarray(tb[p].width, tb[p].height);
        // this conversion is necessary because theora decoder
        // sometimes provides strides that are different than the
        // strides on the input.
        unsigned char *dp = tb[p].data;
        for(int r = 0; r < tb[p].height; ++r)
        {
            memmove(&g[p][r][0], dp, tb[p].width);
#if 0
            for(int c = 0; c < tb[p].width; ++c)
            {
                g[p][r][c] = dp[c];
            }
#endif
            dp += tb[p].stride;
        }
    }

    // upsample the chroma planes
    int ncols[3];
    int nrows[3];
    ncols[0] = tb[0].width;
    nrows[0] = tb[0].height;
    for(int p = 1; p < 3; ++p)
    {
        int tcols = tb[p].width;
        int trows = tb[p].height;
        gray **gt = upsample1_2(g[p], &tcols, &trows);
        pgm_freearray(g[p], tb[p].height);
        g[p] = gt;
        ncols[p] = tcols;
        nrows[p] = trows;
    }


    // note: upsampled chroma may be different in
    // size than original luma, but the extra
    // can be ignored.

    // got all the components, now merge them into
    // a ppm-like rgb thing
    PPMIMG p = ppm_allocarray(ncols[0], nrows[0]);
    JSAMPIMAGE a = new JSAMPARRAY[3];
    a[0] = g[0];
    a[1] = g[1];
    a[2] = g[2];

    ycc_rgb_convert(a, 0, (unsigned char **)p, ncols[0], nrows[0]);

    delete [] a;
    for(int i = 0; i < 3; ++i)
        pgm_freearray(g[i], nrows[i]);

    if(ncols[0] != fcols || nrows[0] != frows)
    {
        PPMIMG p2 = ppm_allocarray(fcols, frows);
        if(fcols > ncols[0] || frows > nrows[0])
            oopanic("can't crop that way");
        for(int i = 0; i < frows; ++i)
        {
            memmove(&p2[i][0], &p[i][0], fcols * sizeof(pixel));
        }
        ppm_freearray(p, nrows[0]);
        p = p2;
        nrows[0] = frows;
        ncols[0] = fcols;
    }

    cols = ncols[0];
    rows = nrows[0];
    vimg = p;
}

void
CDCTheoraDecoderColor::decode_postprocess(DWBYTE *buf, int len)
{
    if(!busy())
        set_busy(1);
    else
        oopanic("decode color busy");
    void *vimg = 0;
    int rows;
    int cols;
    decode_from_stream(buf, len, vimg, cols, rows);
    if(vimg == 0)
    {
        // note: maybe we can do something in display decoded
        // where is puts up a constant pic of "seeking..." or something
        // like that eventually.
        set_busy(0);
        return;
    }
    display_decoded(vimg, cols, rows);
    GRTLOG("display decoded", 0, 0);
    ppm_freearray(vimg, rows);

    set_busy(0);
}

void
CDCTheoraDecoderColor::display_decoded(void *p, int cols, int rows)
{
    pixel **img = (pixel **)p;
    ppm_to_colorview(img, cols, rows, gv_id);
}

void
CDCTheoraDecoderColor::display_info(const char *str)
{
    ::display_info(str, gv_id);
}

#endif

