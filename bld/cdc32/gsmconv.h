#ifndef DWYCO_NO_GSM
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/gsmconv.h 1.1 1999/01/10 16:27:16 dwight Exp $
#ifndef GSMCONV_H
#define GSMCONV_H

#include "audconv.h"
#include "gsm.h"
#ifdef UWB_SAMPLING
extern "C" {
#include "speex/speex_resampler.h"
};
#endif

class GSMConverter : public AudioConverter
{
public:
    GSMConverter(int decom);
    ~GSMConverter();

    virtual int convert(DWBYTE *buf, int len, int user_data, DWBYTE *&buf_out,
                        int &len_out);

private:
    gsm state;
#ifdef UWB_SAMPLING
    SpeexResamplerState *resampler;
#endif
};

#endif
#endif
