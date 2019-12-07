#ifndef DWYCO_NO_GSM
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/gsmconv.cc 1.1 1999/01/10 16:26:27 dwight Exp $
#include "gsmconv.h"
#include <string.h>

GSMConverter::GSMConverter(int decom)
    : AudioConverter(decom)
{
    state = gsm_create();
#ifdef UWB_SAMPLING
    // we have to resample to 8khz
    int err = 0;
    if(decom)
        resampler = speex_resampler_init(1, 8000, UWB_SAMPLE_RATE, 2, &err);
    else
        resampler = speex_resampler_init(1, UWB_SAMPLE_RATE, 8000, 2, &err);
    if(err != RESAMPLER_ERR_SUCCESS)
        oopanic("resampler init failed");
#endif
}

GSMConverter::~GSMConverter()
{
    gsm_destroy(state);
#ifdef UWB_SAMPLING
    speex_resampler_destroy(resampler);
#endif
}


int
GSMConverter::convert(DWBYTE *buf, int len, int , DWBYTE*& out_buf, int& out_len)
{
    if(decompress)
    {
        GRTLOG("gsm decom", 0, 0);
        //buf[0] ^= 0xaa;
        int nframes = len / sizeof(gsm_frame);
        gsm_signal *g = new gsm_signal[160 * nframes];
        out_len = 160 * sizeof(gsm_signal) * nframes;
        out_buf = (DWBYTE *)g;
        // zero it out because if we were recording, it is reasonably
        // likely that the data we get from new will have the
        // raw audio in it, and if there is an error below, that audio
        // could end up being replayed
        memset(g, 0, out_len);
        for(int i = 0; i < nframes; ++i)
        {
            if(gsm_decode(state, buf + i * sizeof(gsm_frame), g + i * 160) == -1)
            {
                // might be garbage, don't want to crash
                return 0;
            }
        }
        GRTLOG("gsm decom-end", 0, 0);
#ifdef UWB_SAMPLING
        spx_uint32_t ilen = out_len / 2;
        spx_uint32_t olen = (ilen * UWB_SAMPLE_RATE) / 8000;
        spx_int16_t *obuf = new spx_int16_t[olen];
        speex_resampler_process_int(resampler, 0, g, &ilen, obuf, &olen);
#if 0
        if(ilen != out_len / 2 || olen != (out_len / 2) * 4)
        {
            oopanic("dec oops");
        }
#endif
        out_buf = (DWBYTE *)obuf;
        out_len = olen * sizeof(obuf[0]);
        delete [] g;
        GRTLOG("gsm decom-resample", 0, 0);
#endif
    }
    else
    {
#ifdef UWB_SAMPLING
// WARNING: ilen and olen must all come out to integral numbers.
// which means the SAMPLE_RATE and 8000 must have some integral
// relationship. it might be possible to use different sample rates
// but i can't be bothered with the complexity at this point, since
// 44100 and 8000 seem to work ok with the current codecs.
        spx_uint32_t ilen = len / 2;
        spx_uint32_t olen = (ilen * 8000) / UWB_SAMPLE_RATE;
        spx_int16_t *obuf = new spx_int16_t[olen];
        speex_resampler_process_int(resampler, 0, (const spx_int16_t *)buf, &ilen, obuf, &olen);
#if 1
        //not sure about interface to resampler, but we *have* to give the
        // gsm converter a full 80ms frame
        if(ilen != len / 2 || olen != 640)
        {
            oopanic("oops");
        }
#endif
        buf = (DWBYTE *)obuf;
        len = olen * sizeof(obuf[0]);
#endif
        int nframes = len / (sizeof(gsm_signal) * 160);
        gsm_byte *g = (gsm_byte *)new gsm_frame[nframes];
        for(int i = 0; i < nframes; ++i)
        {
            gsm_encode(state, ((gsm_signal *)buf) + i * 160, g + sizeof(gsm_frame) * i);
        }
        //g[0] ^= 0xaa;
        out_len = nframes * sizeof(gsm_frame);
        out_buf = (DWBYTE *)g;
#ifdef UWB_SAMPLING
        delete [] obuf;
#endif
    }
    return 1;
}
#endif
