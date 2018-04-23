
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/audmix.cc 1.13 1999/01/10 16:09:46 dwight Checkpoint $
// thread for handling audio mixing
//
// the reason for having this thread is that
// we need finer control over the timing of
// packets as they are sent to the audio driver.
// this reduces the latency between multiple streams
// and gives us a chance to process audio at a higher
// priority level (so if the video codec is chewing up
// a lot of time, we can still keep audio quality
// reasonable.)
//
// the stage that is feeding this thread is assumed to
// have massive buffering (typically at least 200ms)
// to account for internet related delays.
//
// there is a queue for incoming audio packets
// from multiple sources. it is assumed that
// when the producer thread puts something in the
// queue, it wants it played as soon as possible.
// so, the data is mixed immediately with any other
// queued data in preparation for sending to the
// driver.
//
// the driver signals us with an event, so we can keep
// the driver supplied with buffers. we try to keep as
// small a number of buffers as possible queued because
// once the driver has them, there is no way to mix any
// more incoming data.
//
//

#include "dwrtlog.h"
#include "audmix.h"
#ifdef LINUX
#include "audexto.h"
#include "aqextaud.h"
#else
#include "audwin.h"
#include "aqaud.h"
#endif

// set this to the number of buffers you want to try and keep
// playing. results in a delay of this many buffers in the audio
// output. make this as small as possible (4 works ok, but introduces
// a lot of delay, worked well on old computers.)
#define BUFFER_DELAY 4

#define MIXDBG

#ifdef MIXDBG
#define AUDRTLOG(s, i, j) GRTLOGA(s, (i), (j), 0, 0, 0);
#define AUDRTLOGA(s, i, j, k, z) GRTLOGA(s, (i), (j), (k), (z), 0);
#else
#define AUDRTLOG(s, i, j)
#define AUDRTLOGA(s, i, j, k, z)
#endif

extern int Audio_full_duplex;
extern int Talk_override;
extern int All_mute;
long Deferred_release;
// protect us from microsoft
CRITICAL_SECTION Puffm;

AudioMixer::AudioMixer(int ringsize, CRITICAL_SECTION& c) :
    ring(ringsize, 1, 0), ringlen(ringsize, 1, 0), inputq(c), cs(c)
{
    current = 0;
    driver_current = 0;
    num_in_ring = 0;
    buf_done_event = CreateEvent(0, 0, 0, 0);
    driver = 0;
    exit = 0;
    audinp = 0;
    InitializeCriticalSection(&Puffm);
}

AudioMixer::~AudioMixer()
{
    CloseHandle(buf_done_event);
    DeleteCriticalSection(&Puffm);
}

void
AudioMixer::set_driver(AudioOutput *d)
{
    driver = d;
}

void
AudioMixer::done_callback()
{
    SetEvent(buf_done_event);
    /*
     * find the contributors to the buffer and
     * update the currently-playing count for
     * those contributors.
     */
    EnterCriticalSection(&Puffm);
    DwVec<int> &con = buf_contrib[driver_current];
    int n = con.num_elems();
    for(int i = 0; i < n; ++i)
    {
        --num_out[con[i]];
        AUDRTLOG("done cont %d num %d", con[i], num_out[con[i]]);
    }
    con.set_size(0);
    driver_current = (driver_current + 1) % ring.num_elems();
    LeaveCriticalSection(&Puffm);
}

int
AudioMixer::device_bufs_playing(int id)
{
    int tmp;
    CRITICAL_SECTION& cs = Puffm;
    EnterCriticalSection(&cs);
    int i = srcs.index(id);
    if(i == -1)
    {
        tmp = 0;
        goto out;
    }
    tmp = num_out[i];
out:
    LeaveCriticalSection(&cs);
    return tmp;
}

void
AudioMixer::release_bufs()
{
    DWBYTE *free_buf;
    int len;
    int dfree;
    if(!driver)
        return;
    while(driver->device_done(free_buf, len, dfree))
    {
        if(dfree)
            delete [] free_buf;
    }
}

void
AudioMixer::feed_driver()
{
    // if our local bufs contain data to move out
    // and the driver has less than N buffers, feed
    // a few more in to bring it to N.

    int numplaying;

    EnterCriticalSection(&cs);
    if(!driver)
        goto out;
    LeaveCriticalSection(&cs);
    numplaying = driver->device_bufs_playing();
    EnterCriticalSection(&cs);
    AUDRTLOG("feed numplaying %d", numplaying, 0);
    if(numplaying >= BUFFER_DELAY)
        goto out;
    while(numplaying < BUFFER_DELAY && num_in_ring > 0)
    {
        DWBYTE *buf = ring[current];
        int len = ringlen[current];
        int n;
        int i;
        ring[current] = 0;
        ringlen[current] = 0;
        // if we're moving past the last index
        // used by a particular src, make sure
        // we drop the source.
        n = srcs_idx.num_elems();
        for(i = 0; i < n; ++i)
        {
            if(srcs_idx[i] == current)
            {
                AUDRTLOG("reset cont %d from %d", i, srcs_idx[i]);
                srcs_idx[i] = -1;
            }
        }
        AUDRTLOG("feed played %d (out %d)", current, 0);
        current = (current + 1) % ring.num_elems();
        --num_in_ring;
        LeaveCriticalSection(&cs);

        if(All_mute)
        {
            if(audinp)
                audinp->off();
            if(driver)
                driver->on();
        }
        else
        {
            if(driver)
                driver->on();
            if(audinp)
                audinp->on();
        }
#if 0
        else if(!Audio_full_duplex)
        {
            if(!Talk_override)
            {
                if(audinp)
                    audinp->off();
                if(driver)
                    driver->on();
            }
            else
            {
                if(driver)
                    driver->off();
                if(audinp)
                    audinp->on();
            }
        }
        else if(Audio_full_duplex)
        {
            if(driver)
                driver->on();
            if(audinp)
                audinp->on();
        }
#endif
        driver->play(buf, len, 1);
        EnterCriticalSection(&cs);
        ++numplaying;
    }
out:
    LeaveCriticalSection(&cs);

}

int
AudioMixer::unload_q()
{
    // circular q of buffers, for now all one
    // length. when we unload a buffer from the
    // input q, it has an id associated with it
    // to tell who the contributor is. we keep a
    // list that tells which contributor is where in the
    // q. current points to the next buffer that is
    // to be sent out to the driver. all data is assumed
    // to be decoded and in linear 16 format.
    DWBYTE *buf;
    int len;
    int tc;
    int idx;
    int src;
    int ret = 1;

    if(inputq.fifo_buf(buf, len, tc, idx, src) == 0)
        return 0;

    CRITICAL_SECTION& cs = Puffm;
    EnterCriticalSection(&cs);
    // decide where in the q this source needs to be
    // mixed in.
    int i = srcs.index(src);
    if(i == -1)
    {
        // new source
        i = srcs.index(-1);
        if(i == -1)
        {
            srcs.append(src);
            srcs_idx.append(current);
            i = srcs.num_elems() - 1;
        }
        else
        {
            srcs[i] = src;
            srcs_idx[i] = current;
        }
        num_out[i] = 0;
    }
    // get the last index used by this src
    int si = srcs_idx[i];
    // was invalidated (hasn't been contributing for awhile)
    if(si == -1)
    {
        srcs_idx[i] = current;
        si = current;
    }
    if((si + 1) % ring.num_elems() == current)
    {
        AUDRTLOG("can't unload src %d idx %d (full)", src, si);
        ret = 0;
        goto out;
    }
    else
    {
        AUDRTLOG("unload src %d idx %d", src, si);
    }
    if(ring[si] == 0)
    {
        // nothing to mix with, just stash it.
        ring[si] = buf;
        ringlen[si] = len;
        // don't let fifo delete the data
        LeaveCriticalSection(&cs);
        inputq.remove(-1, 0);
        EnterCriticalSection(&cs);
        ++num_in_ring;
        buf_contrib[si].set_size(0);
        buf_contrib[si].append(i);
        AUDRTLOG("no mix", 0, 0);
    }
    else
    {
        // mix with existing data
        short *a = (short *)buf;
        short *ra = (short *)ring[si];
        int alen = len / 2;
        int s;
        for(s = 0; s < alen; ++s)
        {
            int b = ra[s] + a[s];
            if(b < -32768)
                b = -32768;
            else if(b > 32767)
                b = 32767;
            ra[s] = b;
        }
        LeaveCriticalSection(&cs);
        inputq.remove();
        EnterCriticalSection(&cs);
        buf_contrib[si].append(i);
        AUDRTLOG("mix", 0, 0);
    }
    srcs_idx[i] = (si + 1) % ring.num_elems();
    ++num_out[i];
out:
    LeaveCriticalSection(&cs);
    return ret;
}

void
AudioMixer::add(DWBYTE *buf, int len, int id)
{
    inputq.add(buf, len, 0, 0, id);
    AUDRTLOG("added %d len %d", id, len);
}

void
AudioMixer::set_audinp(AudioAcquire *ai)
{
    EnterCriticalSection(&cs);
    audinp = ai;
    LeaveCriticalSection(&cs);
}

int
AudioMixer::device_one_buffer_time()
{
    return driver->device_one_buffer_time();
}

int
AudioMixer::device_buffer_time(int len)
{
    return driver->device_buffer_time(len);
}

void
AudioMixer::audio_mixer_thread()
{
    while(1)
    {
        HANDLE a[2];

        if(exit)
        {
            // with any luck, this should stop any threads
            // the driver might have calling us
            //if(driver)
            //	driver->off();
            return;
        }

        a[0] = inputq.get_event();
        a[1] = buf_done_event;

        int ret = WaitForMultipleObjects(2, a, 0, 100);
        release_bufs();

        if(All_mute)
        {
            if(audinp)
                audinp->off();
            if(driver)
                driver->on();
        }
        else
        {
            if(driver)
                driver->on();
            if(audinp)
                audinp->on();

        }
#if 0
        else if(!Audio_full_duplex)
        {
            if(!Talk_override)
            {
                if(driver && driver->device_bufs_playing() == 0)
                {
                    if(driver)
                        driver->off();
                    if(audinp)
                        audinp->on();
                }
                else
                {
                    if(audinp)
                        audinp->off();
                    if(driver)
                        driver->on();
                }
            }
            else
            {
                if(driver)
                    driver->off();
                if(audinp)
                    audinp->on();
            }
        }
        else if(Audio_full_duplex)
        {
            if(driver)
                driver->on();
            if(audinp)
                audinp->on();
        }
#endif
        if(ret == WAIT_TIMEOUT)
        {
            // do stuff
            continue;
        }
        else if(ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + 1)
        {
            //ret -= WAIT_OBJECT_0;
        }
        else
        {
            oopanic("mixer bad wait return");
        }
        // since the wait is essentially useless for telling
        // us which objects got signalled, we poll other
        // info to determine what to do.
        feed_driver();
        while(inputq.num_elems() > 0 && unload_q())
            ;
    }
}
