
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SD4_H
#define SD4_H

#include "webrtc_vad.h"
#ifdef UWB_SAMPLING
extern "C" {
#include "speex/speex_resampler.h"
};
#endif

class sd4
{
public:
    sd4();
    virtual ~sd4();
    void init(double);

    int sd(void *buf, int len, double sensitivity);
    int do_agc;
    int do_noise_reduction;

private:
    VadInst *pps;
#ifdef UWB_SAMPLING
    SpeexResamplerState *resampler;
#endif

};
#endif
