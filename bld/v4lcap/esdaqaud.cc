
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef LINUX
/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqaud.cc 1.17 1999/01/10 16:09:24 dwight Checkpoint $
 */
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>
#include "aqaud.h"
#include "esdaqaud.h"
#include "dwrtlog.h"
#ifndef USE_ESDREC
#include "esd.h"
#endif
#include "audo.h"

#include "speex/speex_echo.h"
#include "dwyco_rand.h"

extern SpeexEchoState *Echo_state;

void oopanic(const char *);

// not sure if we will need a thread or not, so leave the code in
#define EnterCriticalSection(x)
#define LeaveCriticalSection(x)

EsdAqAud::EsdAqAud(int bs, int nbufs) :
    bufs(nbufs, DWVEC_FIXED, !DWVEC_AUTO_EXPAND),
    dribble_bufs(0, 0, 0), timestamps(0, 0, 0)
{
    streaming = 0;
    nb = nbufs;
    bufsize = bs;
    wavein = -1;
    is_off = 0;
#ifdef USE_ESDREC
    pid = -1;
#endif
    initial_timestamp = dwyco_rand();
    current_offset = 0;
    last_off_time = timeGetTime();
    for(int i = 0; i < nb; ++i)
        bufs[i].data = 0;
    cibuf = cobuf = 0;
    tmpbuf = new DWBYTE[bs];
    b2 = new DWBYTE[bs];
    used = 0;
    amtleft = bufsize;
}

EsdAqAud::~EsdAqAud()
{
    reset();
    for(int i = 0; i < dribble_bufs.num_elems(); ++i)
        delete [] dribble_bufs[i];
    for(int i = 0; i < nb; ++i)
    {
        delete [] bufs[i].data;
    }
    delete [] tmpbuf;
    delete [] b2;
}

void
EsdAqAud::reset()
{
    EnterCriticalSection(&cs);
    if(wavein != -1)
    {
#ifdef USE_ESDREC
        kill(pid, 15);
#endif
        streaming = 0;
        close(wavein);
        wavein = -1;
        last_off_time = timeGetTime();
        cibuf = cobuf = 0;
    }
    LeaveCriticalSection(&cs);
}

int
EsdAqAud::status()
{
    EnterCriticalSection(&cs);
    int tmp = wavein != -1;
    LeaveCriticalSection(&cs);
    return tmp;
}


int
EsdAqAud::init()
{
    EnterCriticalSection(&cs);
    int ret = 1;
    if(wavein != -1)
        goto done;

    // estimate about where we should be in the
    // offset from the wall clock
#ifdef UWB_SAMPLING
    current_offset += (timeGetTime() - last_off_time) * UWB_SAMPLE_RATE / 1000;
#else
    current_offset += (timeGetTime() - last_off_time) * 8000 / 1000;
#endif

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
EsdAqAud::off()
{
    // unload all the buffered data immediately before
    // shutting down the driver.
    EnterCriticalSection(&cs);
    is_off = 1;
    if(wavein == -1)
        goto done;
#ifdef USE_ESDREC
// FUCK ME! this was the soure of about a weeks worth of crashing
// x servers and misdirected debugging. i forgot to comment this out
// when i switched to direct lib calls to esd, and the result was
// (with the pid variable inited to -1) termination of every process
// owned by the runner of the program.
    kill(pid, 15);
#endif
    unload_pipe();
    while(_has_data())
    {
        int len;
        int ts;
        void *buf = _get_data(len, ts);
        delete [] (DWBYTE *)buf;
#if 0
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
        if(len < bufsize)
        {
            // zero pad the incomplete buffer
            memset((DWBYTE *)buf + len, 0, bufsize - len);
            for(int i = 0; i < bufsize - len; i += 2)
            {
                ((short *)buf)[(len + i) / 2] = (dwyco_rand() < RAND_MAX / 2) ? 4 : -4;
            }

        }
        else if(len > bufsize) // hmmm, trashed buffer?
        {
        }
        dribble_bufs.append((DWBYTE *)buf);
        timestamps.append(ts);
#endif
    }

    reset();
done:
    LeaveCriticalSection(&cs);

}

void
EsdAqAud::on()
{
    EnterCriticalSection(&cs);
    is_off = 0;
    if(wavein != -1)
        goto done;
    init();
done:
    need();
    LeaveCriticalSection(&cs);
}

void
EsdAqAud::pass()
{
}

void
EsdAqAud::need()
{
    EnterCriticalSection(&cs);
    if(is_off)
        goto out;
    if(wavein == -1 && !streaming)
    {
#ifdef USE_ESDREC
        int fds[2];

        if(pipe(fds) == -1)
            return;
        if((pid = fork()) == 0)
        {
            close(0);
            close(1);
            close(2);
            close(fds[0]);
            open("/dev/null", O_RDONLY);
            dup2(fds[1], 1);
            open("/dev/null", O_WRONLY);
            execlp("esdrec", "esdrec", "-m", "-r", "8000", (const char *)0);
            _exit(1);
        }
        if(pid == -1)
            return;
        close(fds[1]);
        wavein = fds[0];
#endif
#ifdef UWB_SAMPLING
        wavein = esd_record_stream_fallback(ESD_MONO|ESD_BITS16|ESD_STREAM|ESD_RECORD, UWB_SAMPLE_RATE, 0, 0);
#else
        wavein = esd_record_stream_fallback(ESD_MONO|ESD_BITS16|ESD_STREAM|ESD_RECORD, 8000, 0, 0);
#endif
        // note: some esd apparently can return -2 if there is a
        // problem. it isn't documented anywhere.
        if(wavein < 0)
        {
            wavein = -1;
            goto out;
        }
        streaming = 1;
    }
out:
    LeaveCriticalSection(&cs);
}

int
EsdAqAud::has_data()
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

void
EsdAqAud::unload_pipe()
{
    int len;
    if(ioctl(wavein, FIONREAD, &len) == -1)
    {
        reset();
        return;
    }
    if(len < bufsize || used != 0)
    {
        // ok: we have to pull everything off of the pipe
        // to unblock the other side, and esd gets uppity
        // if it ever gets blocked, it seems to stop capturing.
        if(len > amtleft)
            len = amtleft;
        if(read(wavein, tmpbuf, len) != len)
            return;
        if(len < amtleft)
        {
            memcpy(b2 + used, tmpbuf, len);
            used += len;
            amtleft -= len;
        }
        else
        {
            memcpy(b2 + used, tmpbuf, amtleft);
            audbuf *a = &bufs[cibuf];
            if(bufs[cibuf].data == 0)
                a->data = new DWBYTE[bufsize];
            a->len = bufsize;
            GRTLOG("esd unload pipe %d", cibuf, 0);
            cibuf = (cibuf + 1) % nb;
            if(Echo_state)
            {
                int frames = (a->len / 2) / ECHO_SAMPLES;
                spx_int16_t *s = (spx_int16_t *)b2;
                spx_int16_t *d = (spx_int16_t *)a->data;
                for(int i = 0; i < frames; ++i)
                {
                    speex_echo_capture(Echo_state, s, d);
                    d += ECHO_SAMPLES;
                    s += ECHO_SAMPLES;
                }
            }
            else
                memcpy(a->data, b2, bufsize);
            memcpy(b2, tmpbuf + amtleft, len - amtleft);
            used = len - amtleft;
            amtleft = bufsize - used;
        }

        return;
    }
    while(len >= bufsize)
    {
        audbuf *a = &bufs[cibuf];
        if(bufs[cibuf].data == 0)
            a->data = new DWBYTE[bufsize];
        a->len = bufsize;
        if(read(wavein, a->data, a->len) != a->len)
        {
            return;
        }
        GRTLOG("esd unload pipe2 %d", cibuf, 0);
        if(Echo_state)
        {
            DWBYTE *tmp = new DWBYTE[bufsize];
            int frames = (a->len / 2) / ECHO_SAMPLES;
            spx_int16_t *s = (spx_int16_t *)a->data;
            spx_int16_t *d = (spx_int16_t *)tmp;
            for(int i = 0; i < frames; ++i)
            {
                speex_echo_capture(Echo_state, s, d);
                d += ECHO_SAMPLES;
                s += ECHO_SAMPLES;
            }
            delete [] a->data;
            a->data = tmp;
        }
        cibuf = (cibuf + 1) % nb;
        len -= bufsize;
    }

}

int
EsdAqAud::_has_data()
{
    EnterCriticalSection(&cs);
    int ret = 0;
    if(wavein == -1)
        goto done;
    unload_pipe();
    if(cibuf != cobuf)
    {
        ret = 1;
        goto done;
    }
done:
    LeaveCriticalSection(&cs);
    return ret;
}

void *
EsdAqAud::get_data(int& len, int& out2)
{
    EnterCriticalSection(&cs);
    void *ret;
    if(dribble_bufs.num_elems() > 0)
    {
        ret = dribble_bufs[0];
        dribble_bufs.del(0);
        out2 = timestamps[0];
        timestamps.del(0);
        len = bufsize;
        goto done;
    }
    ret = _get_data(len, out2);
done:
    LeaveCriticalSection(&cs);
    return ret;
}

void *
EsdAqAud::_get_data(int& len, int& out2)
{
    EnterCriticalSection(&cs);
    if(cibuf == cobuf)
    {
        oopanic("audio get not done");
    }
    // unload buffer

    void *retbuf = bufs[cobuf].data;
    int retlen = bufs[cobuf].len;
    bufs[cobuf].data = 0;
    bufs[cobuf].len = 0;
    cobuf = (cobuf + 1) % nb;
    // compute timestamp for the beginning
    // of the block
    out2 = current_offset + initial_timestamp;
    current_offset += retlen;

    len = retlen;
    LeaveCriticalSection(&cs);
    return retbuf;
}



#endif
