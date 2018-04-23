
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/lpc/rcs/sd4.cc 1.3 1999/01/07 03:12:55 dwight Exp $
#include <math.h>
#include "sd4.h"

void oopanic(const char *);

sd4::sd4()
{
    // bad: assumes 20ms frames at 8khz
    pps = 0;
    do_noise_reduction = 0;
    do_agc = 0;
    resampler = 0;
}

sd4::~sd4()
{
    if(pps)
        WebRtcVad_Free(pps);
#ifdef UWB_SAMPLING
    if(resampler)
        speex_resampler_destroy(resampler);
#endif
}

void
sd4::init(double per)
{
    WebRtcVad_Create(&pps);
    WebRtcVad_Init(pps);
#ifdef UWB_SAMPLING
    int err = 0;
    resampler = speex_resampler_init(1, UWB_SAMPLE_RATE, 8000, 7, &err);
    if(err != RESAMPLER_ERR_SUCCESS)
        oopanic("resampler init failed");
#endif
}
#ifdef UWB_SAMPLING
#define SRATE UWB_SAMPLE_RATE
#define FRAMESZ ((int)(UWB_SAMPLE_RATE * 0.01))
#else
#define SRATE 8000
#define FRAMESZ 160
#endif
int
sd4::sd(void *buf, int len, double sensitivity)
{
    if(!pps)
    {
        init(0);
    }

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
    buf = (void *)obuf;
    len = olen * sizeof(obuf[0]);
#endif

    int nframes = len / 160;
    int speech = 0;
    for(int i = 0; i < nframes && !speech; ++i)
    {
        int fs = WebRtcVad_Process(pps, 8000, &((short *)buf)[i * 80], 80);
        //int fs = speex_preprocess(pps, &((short *)buf)[i * FRAMESZ], 0);
        speech |= fs;
    }
    return !speech;
}


