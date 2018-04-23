
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audconv.cc 1.16 1999/01/10 16:09:43 dwight Checkpoint $
 */
//
// this class is really a compressor/decompressor
// wrapper.
// it is a straight conversion class.
//
#include "audconv.h"

AudioConverter::AudioConverter(int decom, int nbufs)
    : AudioOutput(decom, nbufs)
{
}

AudioConverter::~AudioConverter()
{
}

int
AudioConverter::device_status()
{
    return 1;
}

int
AudioConverter::device_close()
{
    return 1;
}


int
AudioConverter::device_init()
{
    return 1;
}

int
AudioConverter::device_output(DWBYTE *buf, int len, int user_data)
{
    DWBYTE *buf_out;
    int len_out;
    convert(buf, len, user_data, buf_out, len_out);
    if(user_data)
    {
        delete [] buf;
    }
    obufs.append(buf_out);
    olens.append(len_out);
    return 1;
}

int
AudioConverter::device_done(DWBYTE*& buf, int& len, int& user_data)
{
    return 0;
}

// this returns the destination data, which will be
// compressed/decompressed, depending on how the
// object was created. the length returned is the
// actual length of the data, which may be smaller
// than the length of the buffer allocated.
// the source buffer is also freed if the user_data
// given to the device_output call is non-zero.
//
int
AudioConverter::device_done2(DWBYTE*& buf, int& len)
{
    if(obufs.num_elems() == 0)
        return 0;
    buf = obufs[0];
    len = olens[0];
    obufs.del(0);
    olens.del(0);
    return 1;
}

int
AudioConverter::device_stop()
{
    return 0;
}

int
AudioConverter::device_reset()
{
    return 1;
}


#undef TEST_AUDOACM
#ifdef TEST_AUDOACM
#include <stdio.h>
#include <stdlib.h>
#include "audwin.h"
main(int argc, char **argv)
{
#if 0
    AudioConverter aw(0, 500);
    char garbage[8192];

    aw.init();
    for(int i = 100; i < 50 * 500; i += 50)
        aw.play_timed(garbage, sizeof(garbage), i, 0);

    DwTimer t;
    t.set_autoreload(0);
    t.set_oneshot(1);
    t.load(60000);
    t.start();
    while(!t.is_expired())
    {
        aw.tick();
        t.tick();
    }
#endif
#define SAMPSZ 2
    typedef DWBYTE SAMP;
#define SAMP_MIN -32768
#define SAMP_MAX 32767
#define NSAMP 1625
#define STEP_MAX 8000
//#define MULCON(x) (((x) * 2050) / 1000)
//#define DIVCON(x) (((x) * 487) / 1000)
#define MULCON(x) (((x) * 2400) / 1000)
#define DIVCON(x) (((x) * 650) / 1000)
#define PREDICT(x) (((x) * 750) / 1000)

    // test by playing a raw file
    AudioOutputWin32PCM16 aw(0, 500);
    AudioConverter ao(1, 500);
    SAMP garbage[NSAMP];

    aw.init();
    ao.init();
    FILE *f = fopen("barf", "rb");
    fread(garbage, sizeof(garbage), 1, f);
    int i = 0;
    while(!feof(f))
    {
        SAMP *buf = new SAMP[NSAMP];
        memcpy(buf, garbage, sizeof(garbage));
        ao.play_timed((DWBYTE*)buf, sizeof(garbage), 0, 1);
        DWBYTE *obuf;
        int len;
        if(!ao.device_done2(obuf, len))
            ::abort();
        aw.play_timed((DWBYTE*)obuf, len, (i * (len / SAMPSZ) * 1000) / 8000 + 1, 1);
        ++i;
        fread(garbage, sizeof(garbage), 1, f);
    }

    DwTimer t;
    t.set_autoreload(0);
    t.set_oneshot(1);
    t.load(60000);
    t.start();
    while(!t.is_expired())
    {
        aw.tick();
        t.tick();
    }
}
#endif
