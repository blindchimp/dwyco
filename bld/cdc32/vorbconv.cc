#ifndef DWYCO_NO_VORBIS
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/gsmconv.cc 1.1 1999/01/10 16:26:27 dwight Exp $
#include "vorbconv.h"
#include "dwstr.h"

#ifdef LINUX
static void gwyn_alloca(size_t) {}
#else
extern "C" void *gwyn_alloca(size_t);
#endif

VorbisConverter::VorbisConverter(int decom, int bit_rate, int sampling_rate)
    : AudioConverter(decom)
{
    // the header stuff doesn't appear to be data dependent, so we can just
    // avoid transmitting it by setting up an identical set of headers
    // in the decoder.
    vorbis_info_init(&vinfo);
    // try CBR for now
    if(vorbis_encode_init(&vinfo, 1, sampling_rate, bit_rate, bit_rate, bit_rate))
        oopanic("bad vorbis encoder setup");
#if 0
    if(vorbis_encode_ctl(&vinfo, OV_ECTL_RATEMANAGE2_SET, 0))
        oopanic("can't turn off rate management");
#endif
    vorbis_analysis_init(&vdsp, &vinfo);
    vorbis_block_init(&vdsp, &vblock);
    vorbis_comment_init(&vcomment);
    vorbis_analysis_headerout(&vdsp, &vcomment, &header, &header_comm, &header_code);
    if(!decom)
    {

    }
    else
    {
        // ok, coder state was written into ogg packets above, so we don't
        // need it here. if i was bold, i would check and see if i could
        // avoid the out-to-ogg, then back-from-ogg for the header stuff, but
        // i have no idea if the coding and decoding state tables and so on
        // are identical.
#if 0
        vorbis_block_clear(&vblock);
        vorbis_dsp_clear(&vdsp);
        vorbis_comment_clear(&vcomment);
        vorbis_info_clear(&vinfo);
#endif

        vorbis_info_init(&s_vinfo);
        vorbis_comment_init(&s_vcomment);

        if(vorbis_synthesis_headerin(&s_vinfo,&s_vcomment,&header) < 0)
            oopanic("bogus vorbis decoder setup1");
        if(vorbis_synthesis_headerin(&s_vinfo,&s_vcomment,&header_comm) < 0)
            oopanic("bogus vorbis decoder setup2");
        if(vorbis_synthesis_headerin(&s_vinfo,&s_vcomment,&header_code) < 0)
            oopanic("bogus vorbis decoder setup3");

        vorbis_synthesis_init(&s_vdsp,&s_vinfo);
        vorbis_block_init(&s_vdsp,&s_vblock);
    }
}

VorbisConverter::~VorbisConverter()
{
    vorbis_block_clear(&vblock);
    vorbis_dsp_clear(&vdsp);
    vorbis_comment_clear(&vcomment);
    vorbis_info_clear(&vinfo);
    if(decompress)
    {
        vorbis_block_clear(&s_vblock);
        vorbis_dsp_clear(&s_vdsp);
        vorbis_comment_clear(&s_vcomment);
        vorbis_info_clear(&s_vinfo);
    }
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

static int
op_in(DWBYTE *&inbuf, ogg_packet *op)
{
    int total_len = *((int *)inbuf);
    inbuf += 4;
    op->b_o_s = *((int *)inbuf);
    inbuf += 4;
    op->e_o_s = *(int *)inbuf;
    inbuf += 4;
    op->granulepos = *(ogg_int64_t *)inbuf;
    inbuf += 8;
    op->packetno = *(ogg_int64_t *)inbuf;
    inbuf += 8;
    op->bytes = total_len - 24;
    op->packet = (unsigned char *)inbuf;
    inbuf += op->bytes;
    return 1;
}

int
VorbisConverter::convert(DWBYTE *buf, int len, int , DWBYTE*& out_buf, int& out_len)
{
    if(decompress)
    {
        DWBYTE *obuf = buf;
        DwString outbuf;
        while(buf - obuf < len)
        {
            ogg_packet op;
            op_in(buf, &op);

            /* we have a packet.  Decode it */
            float **pcm;
            int samples;
            // WARNING: NOT sure if synthesis spanks the packet
            // buffer or not here.
            if(vorbis_synthesis(&s_vblock,&op) == 0)
                vorbis_synthesis_blockin(&s_vdsp, &s_vblock);
            else
            {
                out_len = 0;
                out_buf = new DWBYTE[0];
                return 1;
                //oopanic("vorbis synth failed");
            }
            /*

            **pcm is a multichannel float vector.  In stereo, for
            example, pcm[0] is left, and pcm[1] is right.  samples is
            the size of each channel.  Convert the float values
            (-1.<=range<=1.) to whatever PCM format and write it out */

            while((samples = vorbis_synthesis_pcmout(&s_vdsp, &pcm)) > 0)
            {
                /* convert floats to 16 bit signed ints (host order) and
                   interleave */
                ogg_int16_t *convbuffer = new ogg_int16_t[samples * s_vinfo.channels];
                for(int i=0; i<s_vinfo.channels; i++)
                {
                    ogg_int16_t *ptr=convbuffer+i;
                    float  *mono=pcm[i];
                    for(int j=0; j<samples; j++)
                    {
                        int val=mono[j]*32767.f;

                        /* might as well guard against clipping */
                        if(val>32767) {
                            val=32767;
                        }
                        if(val<-32768) {
                            val=-32768;
                        }
                        *ptr=val;
                        ptr+=s_vinfo.channels;
                    }
                }
                outbuf += DwString((const char *)convbuffer, 0, samples * s_vinfo.channels * sizeof(convbuffer[0]));
                delete [] convbuffer;
                /* tell libvorbis how many samples we actually consumed */
                vorbis_synthesis_read(&s_vdsp, samples);
                gwyn_alloca(0);
            }
        }
        out_len = outbuf.length();
        out_buf = new DWBYTE[out_len];
        memcpy(out_buf, outbuf.c_str(), outbuf.length());
    }
    else
    {
        DwString outbuf;
        // vorbis doesn't appear to have a notion of "frame" like
        // voice codecs, so we just dump the data we get into the
        // codec and go from there.
        float **buffer=vorbis_analysis_buffer(&vdsp, len);
        for(int i = 0; i < len / sizeof(short); ++i)
            buffer[0][i] = ((short *)buf)[i] / 32768.f;
        vorbis_analysis_wrote(&vdsp, len / sizeof(short));

        while(vorbis_analysis_blockout(&vdsp, &vblock) == 1)
        {
// vorbis doesn't really provide CBR, which i tried to set up before.
// this uses the example from encode_example.c for VBR

            ogg_packet op;
#if 0
            // feeble attempt at cbr
            vorbis_analysis(&vblock, &op);
            op_out(&op, outbuf);
            gwyn_alloca(0);
#endif
            vorbis_analysis(&vblock, 0);
            vorbis_bitrate_addblock(&vblock);
            while(vorbis_bitrate_flushpacket(&vdsp,&op))
            {
                op_out(&op, outbuf);
            }
            gwyn_alloca(0);
        }
        out_len = outbuf.length();
        out_buf = new DWBYTE[out_len];
        memcpy(out_buf, outbuf.c_str(), outbuf.length());
    }

    return 1;
}

#endif
