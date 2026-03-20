
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "jcoldec.h"
#include "tpgmdec.h"
#include "dchroma.h"

JPEGDecoderColor::JPEGDecoderColor()
{
#if defined(_Windows) && !defined(__CONSOLE__)
    luma = new TPGMMSWDCTDecoder;
    cb = new JPEGTDecoderChroma(0, (TPGMMSWDCTDecoder *)luma);
    cr = new JPEGTDecoderChroma(1, (TPGMMSWDCTDecoder *)luma);
#else
    auto a = new TPGMMSWDCTDecoder;
    luma = a;
    cb = new JPEGTDecoderChroma(0, a);
    cr = new JPEGTDecoderChroma(1, a);
#endif

}


JPEGDecoderColor::~JPEGDecoderColor()
{
    delete luma;
    delete cb;
    delete cr;
}

