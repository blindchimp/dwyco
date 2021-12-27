
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqaud.cc 1.17 1999/01/10 16:09:24 dwight Checkpoint $
 */
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stdlib.h>
#include "aqaud.h"
#include "dwrtlog.h"
#include "dwyco_rand.h"

[[noreturn]] void oopanic(const char *);

#if 0
AudioAquireWin32::AudioAquireWin32() :
    bufs(0, 0, 0), wavehdrs(0, 0, 0), dribble_bufs(0, 0, 0), timestamps(0, 0, 0)
{
    wavein = 0;
    streaming = 0;
    nb = 0;
    bufsz = 0;
    initial_timestamp = dwyco_rand();
    current_offset = 0;
    last_off_time = timeGetTime();
    //InitializeCriticalSection(&cs);
}
#endif

AudioAquireWin32::AudioAquireWin32(int bs, int nbufs, CRITICAL_SECTION& c) :
    bufs(0, 0, 0), wavehdrs(0, 0, 0), dribble_bufs(0, 0, 0), timestamps(0, 0, 0),
    cs(c)
{
    wavein = 0;
    streaming = 0;
    nb = nbufs;
    bufsz = bs;
    initial_timestamp = dwyco_rand();
    current_offset = 0;
    last_off_time = timeGetTime();
    aec = 0;
    //InitializeCriticalSection(&cs);
}

AudioAquireWin32::~AudioAquireWin32()
{
    reset();
    for(int i = 0; i < dribble_bufs.num_elems(); ++i)
        delete [] dribble_bufs[i];
    //DeleteCriticalSection(&cs);
}

void
AudioAquireWin32::reset()
{
    EnterCriticalSection(&cs);
    if(wavein)
    {
        waveInReset(wavein);
        int n = wavehdrs.num_elems();
        for(int i = 0; i < n; ++i)
        {
            volatile WAVEHDR *w = wavehdrs[i];
            while(!(w->dwFlags & WHDR_DONE))
                ; //spin.. dangerous but docs say buffers get returned...
            waveInUnprepareHeader(wavein, (WAVEHDR *)w, sizeof(*w));
            delete [] w->lpData;
            delete w;
        }
        streaming = 0;
        waveInClose(wavein);
        wavein = 0;
        last_off_time = timeGetTime();
    }

    LeaveCriticalSection(&cs);
}

int
AudioAquireWin32::status()
{
    EnterCriticalSection(&cs);
    int tmp = wavein != 0;
    LeaveCriticalSection(&cs);
    return tmp;
}


int
AudioAquireWin32::init(int bufsize, int nbufs, HWND win)
{
    EnterCriticalSection(&cs);
    int ret = 1;
    if(wavein)
        goto done;

    MMRESULT res;
    WAVEFORMATEX format;
    WAVEHDR *w;
    int i;

    init_format(format);

    res = waveInOpen(&wavein, WAVE_MAPPER, &format, (DWORD)win, 0,
                     win ? CALLBACK_WINDOW : CALLBACK_NULL);
    if(res != MMSYSERR_NOERROR)
    {
        ret = 0;
        goto done;
    }

    bufs.set_size(nbufs);
    wavehdrs.set_size(nbufs);

    for(i = 0; i < nbufs; ++i)
    {
        w = new WAVEHDR;
        w->lpData = new char[bufsize];
        w->dwBufferLength = bufsize;
        w->dwFlags = 0;
        if(waveInPrepareHeader(wavein, w, sizeof(*w)) != MMSYSERR_NOERROR)
        {
            goto fail;
        }
        if(waveInAddBuffer(wavein, w, sizeof(*w)) != MMSYSERR_NOERROR)
        {
            goto fail;
        }
        wavehdrs[i] = w;
    }
    // remember bufsize and nbufs
    bufsz = bufsize;
    nb = nbufs;

    // estimate about where we should be in the
    // offset from the wall clock
    current_offset += (timeGetTime() - last_off_time) * format.nSamplesPerSec / 1000;

    goto done;
fail:
    ret = 0;
    // we don't try to clean up partial results too
    // much here because there is something really wrong
    // with the system at this point...
    waveInClose(wavein);
    wavein = 0;
    bufs.set_size(0);
    wavehdrs.set_size(0);
done:
    LeaveCriticalSection(&cs);
    return ret;
}

// these functions allow support for half-duplex audio
// devices. note that data can still dribble out to the
// application after the device has been "off"ed.
// an app should continue to poll "has_data" even
// after it has called off.
void
AudioAquireWin32::off()
{
    // unload all the buffered data immediately before
    // shutting down the driver.
    EnterCriticalSection(&cs);
    if(!wavein)
        goto done;
    waveInStop(wavein);
    while(_has_data())
    {
        int len;
        int ts;
        void *buf = _get_data(len, ts);
        // oops, doesn't always work. some codecs don't like getting flat
        // liners... so we add a tiny bit of noise to avoid problems.
        for(int i = 0; i < len; i += 2)
        {
            int a;
            a = (((short *)buf)[i / 2] + (dwyco_rand() < RAND_MAX / 2) ? 4 : -4);
            if(a < -32767)
                ((short *)buf)[i / 2] = -32767;
            else if(a > 32767)
                ((short *)buf)[i / 2] = 32767;
            ((short *)buf)[i / 2] = a;

        }
        if(len < bufsz)
        {
            // zero pad the incomplete buffer
            memset((DWBYTE *)buf + len, 0, bufsz - len);
            for(int i = 0; i < bufsz - len; i += 2)
            {
                ((short *)buf)[(len + i) / 2] = (dwyco_rand() < RAND_MAX / 2) ? 4 : -4;
            }

        }
        else if(len > bufsz) // hmmm, trashed buffer?
        {
        }
        dribble_bufs.append((DWBYTE *)buf);
        timestamps.append(ts);
    }

    reset();
done:
    LeaveCriticalSection(&cs);

}

void
AudioAquireWin32::on()
{
    EnterCriticalSection(&cs);
    if(wavein)
        goto done;
    init(bufsz, nb);
done:
    need();
    LeaveCriticalSection(&cs);
}

void
AudioAquireWin32::pass()
{
}

void
AudioAquireWin32::need()
{
    EnterCriticalSection(&cs);
    if(wavein && !streaming)
    {
        if(waveInStart(wavein) != MMSYSERR_NOERROR)
            return;
        streaming = 1;
    }
    LeaveCriticalSection(&cs);
}

int
AudioAquireWin32::has_data()
{
    EnterCriticalSection(&cs);
    int ret = 1;
    if(dribble_bufs.num_elems() > 0)
        goto done;
    ret = _has_data();
done:
    LeaveCriticalSection(&cs);
    return ret;
}

int
AudioAquireWin32::_has_data()
{
    EnterCriticalSection(&cs);
    int ret = 0;
    WAVEHDR *w;
    if(!wavein)
        goto done;
    w = wavehdrs[0];
    if(w->dwFlags & WHDR_DONE)
    {
        ret = 1;
        goto done;
    }
done:
    LeaveCriticalSection(&cs);
    return ret;
}

void *
AudioAquireWin32::get_data(int& len, int& out2)
{
    EnterCriticalSection(&cs);
    void *ret;
    if(dribble_bufs.num_elems() > 0)
    {
        ret = dribble_bufs[0];
        dribble_bufs.del(0);
        out2 = timestamps[0];
        timestamps.del(0);
        len = bufsz;
        goto done;
    }
    ret = _get_data(len, out2);

done:

    LeaveCriticalSection(&cs);
    return ret;
}

void *
AudioAquireWin32::_get_data(int& len, int& out2)
{
    EnterCriticalSection(&cs);
    WAVEHDR *w = wavehdrs[0];
    if(!(w->dwFlags & WHDR_DONE))
    {
        oopanic("audio get not done");
    }
#if 0
    // unload buffer
    MMTIME mt;
    mt.wType = TIME_SAMPLES;
    waveInGetPosition(wavein, &mt, sizeof(mt));
    // this actually happened in the field in 5/2007
    if(mt.wType != TIME_SAMPLES)
        oopanic("input assholes");

#ifdef UWB_SAMPLING
    GRTLOG("input pos %d samples %d", mt.u.sample, (int)(mt.u.sample / (UWB_SAMPLE_RATE / 1000) - timeGetTime()));
#else
    GRTLOG("input pos %d samples %d", mt.u.sample, (int)(mt.u.sample / 8 - timeGetTime()));
#endif
#endif
    wavehdrs.del(0);
    void *retbuf = w->lpData;
    int retlen = w->dwBytesRecorded;
    // compute timestamp for the beginning
    // of the block
    out2 = current_offset + initial_timestamp;
    current_offset += retlen;

    waveInUnprepareHeader(wavein, w, sizeof(*w));
    w->lpData = new char[w->dwBufferLength];
    waveInPrepareHeader(wavein, w, sizeof(*w));
    waveInAddBuffer(wavein, w, sizeof(*w));
    wavehdrs.append(w);
    len = retlen;
    LeaveCriticalSection(&cs);
    return retbuf;
}


int
AudioAquireWin32PCM8::init_format(WAVEFORMATEX& format)
{
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = 8000;
    format.wBitsPerSample = 8;
    format.nBlockAlign = 1;
    format.nAvgBytesPerSec = 8000;
    format.cbSize = 0;
    return 1;
}


int
AudioAquireWin32PCM16::init_format(WAVEFORMATEX& format)
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
AudioAquireWin32GSM::init_format(WAVEFORMATEX& format)
{
    format.wFormatTag = WAVE_FORMAT_GSM610;
    format.nChannels = 1;
    format.nSamplesPerSec = 8000;
    format.wBitsPerSample = 8;
    format.nBlockAlign = 33;
    format.nAvgBytesPerSec = 8000;
    format.cbSize = sizeof(GSM610WAVEFORMAT) - sizeof(WAVEFORMATEX);
    return 1;
}

#undef TEST_AQAUD
#ifdef TEST_AQAUD
#include <stdio.h>
main(int argc, char **argv)
{
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
#define PREDICT(x) (((x) * 500) / 1000)

    AudioAquireWin32PCM16 a;

    if(!a.init(10, NSAMP * SAMPSZ))
    {
        fprintf(stderr, "can't init");
        exit(1);
    }

    a.need();
    FILE *f = fopen("barf", "wb");
    for(int i = 0; i < 4; )
    {
        if(a.has_data())
        {
            int len;
            int foo;
            SAMP *buf = (SAMP *)a.get_data(len, foo);
            int j;
            for(j = 1; j < NSAMP - 1; ++j)
                buf[j] = (buf[j - 1] + 2 * buf[j] + buf[j + 1]) / 4;

            int last_samp = *buf;
            int last_out = 0;
            int stepsize = 10;
            for(j = 1; j < NSAMP; ++j)
            {
                //printf("%d\n", stepsize);
                int tmp = buf[j] - last_samp;
                if(tmp < 0)
                    buf[j] = 0;
                else
                    buf[j] = 1;
                last_samp += ((tmp < 0) ? -stepsize : stepsize);
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
            }
            fwrite((char *)buf, len, 1, f);
            ++i;
            delete [] buf;
        }
    }
}
#endif
