
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef DWYCO_USE_SPEEX
#ifndef SPXCONV_H
#define SPXCONV_H

#include "audconv.h"
#include "speex/speex.h"
#include "speex/speex_preprocess.h"

class SpeexConverter : public AudioConverter
{
public:
    SpeexConverter(int decom, int bit_rate, int sample_rate, int complexity = 3);
    ~SpeexConverter();

    virtual int convert(DWBYTE *buf, int len, int user_data, DWBYTE *&buf_out,
                        int &len_out);

private:
    SpeexBits bits;
    void *state;
    int frame_size;
    int bitrate;

    //SpeexPreprocessState *pstate;

};

#endif
#endif
