
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audwin.cc 1.19 1999/01/10 16:09:28 dwight Checkpoint $
 */
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include "audwin.h"
#include "audmix.h"
#include "dwrtlog.h"

static int Resetting;

AudioOutputWin32::AudioOutputWin32(CRITICAL_SECTION& c, int decom, int nbufs)
    : AudioOutput(decom, nbufs), cs(c)
{
    waveout = 0;
    silence = 0;
    silence_len = 0;
    num_out = 0;
    aec = 0;
    //InitializeCriticalSection(&cs);
}

void
AudioOutputWin32::set_aec(int i)
{
    aec = i;
}
void
AudioOutputWin32::set_mixer(AudioMixer *m)
{
    EnterCriticalSection(&cs);
    mixer = m;
    LeaveCriticalSection(&cs);
}

AudioOutputWin32::~AudioOutputWin32()
{
    device_reset();
    if(waveout)
        waveOutClose(waveout);
    delete [] silence;
    //DeleteCriticalSection(&cs);
}

int
AudioOutputWin32::device_status()
{
    return waveout != 0;
}

int
AudioOutputWin32::device_close()
{
    device_reset();
    if(waveout)
    {
        waveOutClose(waveout);
        waveout = 0;
    }
    return 1;
}

int
AudioOutputWin32::device_bufs_playing()
{
    int a = num_out;
    return a;
}

static
void CALLBACK
cb_dispatch(HWAVEOUT hwo, UINT msg, DWORD obj, DWORD p1, DWORD p2)
{
    AudioOutputWin32 *o = (AudioOutputWin32 *)obj;
    o->callback(hwo, msg, p1, p2);
}

void
AudioOutputWin32::callback(HWAVEOUT hwo, UINT msg, DWORD p1, DWORD p2)
{
    switch(msg)
    {
    case WOM_DONE:
        InterlockedDecrement(&num_out);
        if(mixer)
            mixer->done_callback();
        break;
    case WOM_CLOSE:
    case WOM_OPEN:
    default:
        ;
    }
}

int
AudioOutputWin32::device_init()
{
    if(waveout)
        return 1;
    MMRESULT res;
    WAVEFORMATEX format;
    init_format(format);

    res = waveOutOpen(&waveout, WAVE_MAPPER, &format,
                      (DWORD)cb_dispatch, (DWORD)this, CALLBACK_FUNCTION);

    if(res != MMSYSERR_NOERROR)
    {
        waveout = 0;
        return 0;
    }
    InterlockedExchange(&num_out, 0);
    return 1;
}

int
AudioOutputWin32::device_play_silence()
{
    return device_output(silence, silence_len, 0);
}

int
AudioOutputWin32::device_output(DWBYTE *buf, int len, int user_data)
{
    if(!waveout)
    {
        // just drop it
        if(user_data)
            delete [] buf;
        return 1;
    }
    WAVEHDR *h = new WAVEHDR;

    h->lpData = (LPSTR)buf;
    h->dwBufferLength = len;
    h->dwUser = user_data;
    h->dwFlags = 0;
    h->dwLoops = 0;
    MMRESULT res = waveOutPrepareHeader(waveout, h, sizeof(*h));
    if(res != MMSYSERR_NOERROR)
    {
        delete h;
        return 0;
    }
    res = waveOutWrite(waveout, h, sizeof(*h));
    if(res != MMSYSERR_NOERROR)
    {
        delete h;
        return 0;
    }
    //EnterCriticalSection(&cs);
    //++num_out;
    //LeaveCriticalSection(&cs);
    InterlockedIncrement(&num_out);
    wavehdrs.append(h);
    return 1;
}

int
AudioOutputWin32::device_done(DWBYTE*& buf, int& len, int& user_data)
{
    if(wavehdrs.num_elems() ==  0)
        return 0;
    WAVEHDR *h = wavehdrs[0];
    if(!(h->dwFlags & WHDR_DONE))
        return 0;
    buf = (DWBYTE *)h->lpData;
    len = h->dwBufferLength;
    user_data = h->dwUser;

    waveOutUnprepareHeader(waveout, h, sizeof(*h));
    delete h;
    wavehdrs.del(0);
    return 1;
}

int
AudioOutputWin32::device_stop()
{
    if(waveout)
        waveOutPause(waveout);
    return 1;
}

int
AudioOutputWin32::device_reset()
{
    if(!waveout)
        return 1;
    waveOutReset(waveout);
    // unload any pending buffers, dangerous to
    // spin, but docs say all buffers get returned...
    DWBYTE *buf;
    int len;
    int dofree;
    while(device_done(buf, len, dofree))
        if(dofree)
        {
            delete [] buf;
        }
    //num_out = 0;
    return 1;
}

AudioOutputWin32PCM16::AudioOutputWin32PCM16(CRITICAL_SECTION& c, int decom, int nbufs, int bufsz)
    : AudioOutputWin32(c, decom, nbufs)
{
    // create silence buffer
    silence = new DWBYTE[bufsz];
    silence_len = bufsz;
    memset(silence, 0, silence_len);
}

int
AudioOutputWin32PCM16::init_format(WAVEFORMATEX& format)
{
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
#ifdef UWB_SAMPLING
    format.nSamplesPerSec = UWB_SAMPLE_RATE;
    format.nAvgBytesPerSec = UWB_SAMPLE_RATE * 2;
#else
    format.nSamplesPerSec = 8000;
    format.nAvgBytesPerSec = 16000;
#endif
    format.wBitsPerSample = 16;
    format.nBlockAlign = 2;

    format.cbSize = 0;
    return 1;
}

int
AudioOutputWin32PCM16::device_buffer_time(int sz)
{
    WAVEFORMATEX f;
    init_format(f);
    int s = (((sz * 8) / f.wBitsPerSample) * 1000) / f.nSamplesPerSec;
    return s;
}

int
AudioOutputWin32PCM16::device_one_buffer_time()
{
    return device_buffer_time(silence_len);
}


#undef TEST_AUDWIN
#ifdef TEST_AUDWIN
#include <stdio.h>
#include <stdlib.h>
main(int argc, char **argv)
{
#if 0
    AudioOutputWin32 aw(0, 500);
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
    typedef short SAMP;
#define SAMP_MIN -32768
#define SAMP_MAX 32767
#define NSAMP 8000
#define STEP_MAX 8000
//#define MULCON(x) (((x) * 2050) / 1000)
//#define DIVCON(x) (((x) * 487) / 1000)
#define MULCON(x) (((x) * 2400) / 1000)
#define DIVCON(x) (((x) * 650) / 1000)
#define PREDICT(x) (((x) * 750) / 1000)

    // test by playing a raw file
    AudioOutputWin32PCM16 aw(0, 500);
    SAMP garbage[NSAMP];

    aw.init();
    FILE *f = fopen("barf", "rb");
    fread(garbage, sizeof(garbage), 1, f);
    int i = 0;
    while(!feof(f))
    {
        SAMP *buf = new SAMP[8000];
        //memcpy(buf, garbage, sizeof(garbage));
        swab((char *)garbage, (char *)buf, sizeof(garbage));
#if 0
        int last_samp = buf[0];
        int stepsize = 10;
        int last_out = 0;
        for(int j = 1; j < NSAMP; ++j)
        {
            printf("%d\n", stepsize);
            if(buf[j])
                last_samp = last_samp + stepsize;
            else
                last_samp = last_samp - stepsize;
            if(last_out == buf[j])
                stepsize = MULCON(stepsize);
            else
                stepsize = DIVCON(stepsize);
            if(stepsize < 1)
                stepsize = 1;
            else if(stepsize > STEP_MAX)
                stepsize = STEP_MAX;
            last_out = buf[j];
            if(last_samp < SAMP_MIN)
                last_samp = SAMP_MIN;
            else if(last_samp > SAMP_MAX)
                last_samp = SAMP_MAX;
            buf[j] = last_samp;
        }
        for(j = 1; j < NSAMP - 1; ++j)
            buf[j] = (buf[j - 1] + 2 * buf[j] + buf[j + 1]) / 4;
#endif
        aw.device_output((DWBYTE*)buf, sizeof(garbage), 1);
        //aw.play_timed((DWBYTE*)buf, sizeof(garbage), (i * NSAMP * 1000) / 8000 + 1, 1);
        ++i;
        fread(garbage, sizeof(garbage), 1, f);
    }

#if 0
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
    DWBYTE *buf;
    int len;
    int ud;
    while(1);
    //while(aw.device_done(buf, len, ud))
    //	;
}
#endif
