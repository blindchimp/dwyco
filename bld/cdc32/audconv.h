
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audconv.h 1.16 1999/01/10 16:10:56 dwight Checkpoint $
 */
#ifndef AUDCONV_H
#define AUDCONV_H
#include "dwvec.h"
#include "dwvecp.h"
#include "audout.h"

class AudioConverter : public AudioOutput
{
public:
    AudioConverter(int decom = 0, int bufs = DEFAULT_AUDIO_BUFS);
    ~AudioConverter();

    virtual int convert(DWBYTE *buf, int len, int user_data, DWBYTE *&buf_out,
                        int &len_out) = 0;
    virtual int device_init();
    virtual int device_output(DWBYTE *buf, int len, int user_data);
    virtual int device_done(DWBYTE*& buf, int& len, int& do_free) ;
    virtual int device_stop();
    virtual int device_reset();
    virtual int device_close();
    virtual int device_status();
    virtual int device_buffer_time(int) {
        return 0;
    }
    virtual int device_one_buffer_time() {
        return 0;
    }
    virtual int device_play_silence() {
        return 0;
    }

    virtual int device_done2(DWBYTE*& outbuf, int& len);


private:
    DwVecP<DWBYTE> obufs;
    DwVec<int> olens;

};

#endif
