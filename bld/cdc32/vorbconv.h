#ifndef DWYCO_NO_VORBIS
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/gsmconv.h 1.1 1999/01/10 16:27:16 dwight Exp $
#ifndef VORBCONV_H
#define VORBCONV_H

#include "audconv.h"
#include "vorbis/vorbisenc.h"
#include "ogg/ogg.h"

class VorbisConverter : public AudioConverter
{
public:
    VorbisConverter(int decom, int bit_rate = 64000, int sample_rate = 32000);
    ~VorbisConverter();

    virtual int convert(DWBYTE *buf, int len, int user_data, DWBYTE *&buf_out,
                        int &len_out);

private:
    // these are for analysis
    vorbis_info vinfo;
    vorbis_dsp_state vdsp;
    vorbis_block vblock;
    vorbis_comment vcomment;

    // these are for synthesis
    vorbis_info s_vinfo;
    vorbis_dsp_state s_vdsp;
    vorbis_block s_vblock;
    vorbis_comment s_vcomment;

    // initialize these at create time, then send them when the first
    // encoder request is had (or fill them in from the first decoded
    // data.)
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    void *state;
    int frame_size;
    int bitrate;

};

#endif
#endif
