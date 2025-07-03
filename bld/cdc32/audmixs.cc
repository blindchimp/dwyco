
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audmixs.cc 1.13 1999/01/10 16:09:46 dwight Checkpoint $
 */
//
// this class is a pseudo device that knows how to
// send audio buffers to a mixing thread.
// it is assumed there might be multiple instances
// of these objects all feeding data into one mixer.
// the mixer has control of the device that is used for
// output. this class can mimic some of the device
// related functionality, so for example, if a
// single object wants to inhibit sending data to the
// mixer, it can call the usual base class functions to
// turn the channel on and off.
//
// ca. 7/2007
// this class now provides some re-packetizing of raw audio data.
// previously, it was assumed that all feeders would send us the same
// uniformly sized packets (AUDBUFLEN bytes), which was ok, since
// we had only one CBR codec (gsm610) and speex could be made to perform
// in the same way. with the advent of vorbis, we get more or less
// random sizes of packets from the decoder. since the mixer really
// needs fixed size packets in order to figure out what audio needs to
// be mixed together (say we have music playing via vorbis, and want to play
// a qm that was coded with gsm610), this class now examines the packets
// coming in, and only feeds in full AUDBUFLEN sized packets.

#include "audmixs.h"
#include "audmix.h"
#include "audo.h"
#include "dwrtlog.h"
#ifdef LINUX
#include <string.h>
#endif

int AudioMixSend::Id;
// for now, we assume there is one mixer in the
// system. this makes it easier during
// tear-down, since we don't have to go finding
// everyone that references a mixer if the mixer
// is deleted.
extern AudioMixer *volatile TheAudioMixer;
extern CRITICAL_SECTION Audio_lock;

AudioMixSend::AudioMixSend(AudioMixer *m, int nbufs, int bufsz)
#ifdef LINUX
#else
    : AudioOutputWin32PCM16(Audio_lock, 0, nbufs, bufsz)
#endif
{
    myid = Id++;
    //mixer = m;
    buf_leftover = 0;
    len_leftover = 0;
}

AudioMixSend::~AudioMixSend()
{
    if(buf_leftover)
        delete [] buf_leftover;
    buf_leftover = 0;
}

void
AudioMixSend::callback(HWAVEOUT hwo, UINT msg, DWORD_PTR p1, DWORD_PTR p2)
{
    oopanic("mixsend callback?");
}

int
AudioMixSend::device_bufs_playing()
{
    if(!TheAudioMixer)
    {
        return 0;
    }
    int tmp = TheAudioMixer->device_bufs_playing(myid);
    return tmp;
}

int
AudioMixSend::device_status()
{
    return 1;
}

int
AudioMixSend::device_close()
{
    return 1;
}


int
AudioMixSend::device_init()
{
    return 1;
}

int
AudioMixSend::device_output(DWBYTE *buf, int len, int user_data)
{
    EnterCriticalSection(&Audio_lock);
    if(!TheAudioMixer)
    {
        if(user_data)
            delete [] buf;
        if(buf_leftover)
        {
            delete [] buf_leftover;
            buf_leftover = 0;
            len_leftover = 0;
        }
        LeaveCriticalSection(&Audio_lock);
        return 1;
    }

    if(user_data == 0) // not transfering to us
    {
        DWBYTE *b2 = new DWBYTE[len];
        memmove(b2, buf, len);
        buf = b2;
    }
    DWBYTE *newbuf = 0;
    if(buf_leftover)
    {
        // create a new buffer with the leftover from the
        // previous call at the beginning
        newbuf = new DWBYTE[len_leftover + len];
        memmove(newbuf, buf_leftover, len_leftover);
        memmove(newbuf + len_leftover, buf, len);
        delete [] buf;
        buf = newbuf;
        len = len_leftover + len;
        delete [] buf_leftover;
        buf_leftover = 0;
        len_leftover = 0;
        newbuf = 0;
    }
    while(len >= AUDBUF_LEN)
    {
        if(len > AUDBUF_LEN)
        {
            newbuf = new DWBYTE[AUDBUF_LEN];
            memmove(newbuf, buf, AUDBUF_LEN);
            TheAudioMixer->add(newbuf, AUDBUF_LEN, myid);
            // note: the mixer took ownership of newbuf
            newbuf = new DWBYTE[len - AUDBUF_LEN];
            memmove(newbuf, buf + AUDBUF_LEN, len - AUDBUF_LEN);
            delete [] buf;
            buf = newbuf;
            len -= AUDBUF_LEN;
            newbuf = 0;
        }
        else
        {
            TheAudioMixer->add(buf, len, myid);
            len -= AUDBUF_LEN;
            buf = 0;
        }
    }
    buf_leftover = buf;
    len_leftover = len;
    GRTLOG("mixs leftover %d len %d", myid, len_leftover);

    LeaveCriticalSection(&Audio_lock);
    return 1;
}

int
AudioMixSend::device_done(DWBYTE*& buf, int& len, int& user_data)
{
    return 0;
}

int
AudioMixSend::device_done2(DWBYTE*& buf, int& len)
{
    return 0;
}

int
AudioMixSend::device_stop()
{
    return 0;
}

int
AudioMixSend::device_reset()
{
    return 1;
}


int
AudioMixSend::device_buffer_time(int len)
{
    if(!TheAudioMixer)
        return 0;
    return TheAudioMixer->device_buffer_time(len);
}

int
AudioMixSend::device_one_buffer_time()
{
    if(!TheAudioMixer)
        return 0;
    return TheAudioMixer->device_one_buffer_time();
}

int
AudioMixSend::device_play_silence()
{
    // assume 0 is silence, bad, but ok for now
    // this whole play silence thing isn't right for
    // variable length stuff. need to reexamine this whole thing
    // and maybe get rid of it (the assumption that we are playing
    // a fixed size block of silence was inherited from the old
    // days when we did jitter buffering in blocks.)

    DWBYTE a[AUDBUF_LEN];
    memset(a, 0, AUDBUF_LEN);
    device_output(a, AUDBUF_LEN, 0);
    return 1;
}

