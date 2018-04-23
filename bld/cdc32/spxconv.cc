
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef DWYCO_USE_SPEEX
// $Header: g:/dwight/repo/cdc32/rcs/gsmconv.cc 1.1 1999/01/10 16:26:27 dwight Exp $
#include "spxconv.h"

SpeexConverter::SpeexConverter(int decom, int bit_rate, int sampling_rate, int complexity)
    : AudioConverter(decom)
{
    speex_bits_init(&bits);
    if(!decom)
    {
        switch(sampling_rate)
        {
        case 8000:
            state = speex_encoder_init(&speex_nb_mode);
            break;
        case 16000:
            state = speex_encoder_init(&speex_wb_mode);
            break;
        case 32000:
        case 44100:
            state = speex_encoder_init(&speex_uwb_mode);
            break;
        default:
            oopanic("sample rate must be 8000, 16000, 32000, 44100");

        }

        speex_encoder_ctl(state, SPEEX_SET_BITRATE, &bit_rate);
        speex_encoder_ctl(state, SPEEX_GET_BITRATE, &bitrate);
        speex_encoder_ctl(state, SPEEX_SET_SAMPLING_RATE, &sampling_rate);
        speex_encoder_ctl(state, SPEEX_GET_FRAME_SIZE, &frame_size);
        speex_encoder_ctl(state, SPEEX_SET_COMPLEXITY, &complexity);

#if 0
        pstate = speex_preprocess_state_init(frame_size, 8000);
        int on = 1;
        int off = 0;
        speex_preprocess_ctl(pstate, SPEEX_PREPROCESS_SET_DENOISE, &on);
        speex_preprocess_ctl(pstate, SPEEX_PREPROCESS_SET_AGC, &on);
        //speex_preprocess_ctl(pstate, SPEEX_PREPROCESS_SET_DEREVERB, &on);
        //speex_preprocess_ctl(pstate, SPEEX_PREPROCESS_SET_VAD, &on);
#endif

    }
    else
    {
        switch(sampling_rate)
        {
        case 8000:
            state = speex_decoder_init(&speex_nb_mode);
            break;
        case 16000:
            state = speex_decoder_init(&speex_wb_mode);
            break;
        case 32000:
        case 44100:
            state = speex_decoder_init(&speex_uwb_mode);
            break;
        default:
            oopanic("decode sample rate must be 8000, 16000, 32000, 44100");

        }
        speex_decoder_ctl(state, SPEEX_GET_FRAME_SIZE, &frame_size);
    }
}

SpeexConverter::~SpeexConverter()
{
    speex_bits_destroy(&bits);
    if(!decompress)
    {
        speex_encoder_destroy(state);
    }
    else
    {
        speex_decoder_destroy(state);
    }
}


int
SpeexConverter::convert(DWBYTE *buf, int len, int , DWBYTE*& out_buf, int& out_len)
{
    if(decompress)
    {
        DWBYTE *ib = buf;
        float *fob = new float[frame_size];
        short *ob = new short[frame_size * 4]; // NOTE: fix this, don't really know everything is 4 frames
        int f = 0;
        while(ib - buf < len)
        {
            short nb = *(short *)ib;
            ib += sizeof(short);
            speex_bits_read_from(&bits, (char *)ib, nb);
            ib += nb;
            if((long)ib & 1)
                ++ib;
            speex_decode(state, &bits, fob);
            for(int i = 0; i < frame_size; ++i)
                ob[f * frame_size + i] = fob[i];
            ++f;

        }
        delete [] fob;
        out_buf = (DWBYTE *)ob;
        out_len = frame_size * 4 * sizeof(ob[0]);
    }
    else
    {
        int nframes = len / (frame_size * sizeof(short));
        if(nframes == 0)
            oopanic("speex no frames");
        float *inf = new float[frame_size];
        int blen = len + nframes * sizeof(short);
        DWBYTE *ob = new DWBYTE[blen]; // use len since it wouldn't be much of a compressor if it produced more output than input
        DWBYTE *tob = ob;
        for(int i = 0; i < nframes; ++i)
        {
            // warning: preprocess spanks samples in place, have to check
            // to see if this might be a problem.
            //speex_preprocess(pstate, &((short *)buf)[frame_size * i], 0);
            // note: preprocessing is now done outside the converter, which
            // is a better place to do it anyways.
            for(int j = 0; j < frame_size; ++j)
                inf[j] = ((short *)buf)[i * frame_size + j];
            speex_bits_reset(&bits);
            speex_encode(state, inf, &bits);
            int nb = speex_bits_nbytes(&bits);
            *(short *)tob = nb;
            tob += sizeof(short);
            if(speex_bits_write(&bits, (char *)tob, len - (tob - ob)) != nb)
                oopanic("speex overflow");
            tob += nb;
            // move tob up to the next short boundary just in case
            // we have a goofy machine that doesn't like unaligned refs
            if((long)tob & 1)
                ++tob;
            // +2 to account for the eventual short assignment in the
            // next iteration.
            if(tob + 2 >= &ob[blen])
                oopanic("speex overflow2");
        }
        //g[0] ^= 0xaa;
        // create a new buffer of just the right size
        out_len = tob - ob;
        out_buf = new DWBYTE[out_len];
        memcpy(out_buf, ob, out_len);
        delete [] ob;
        delete [] inf;
    }
    return 1;
}
#endif
