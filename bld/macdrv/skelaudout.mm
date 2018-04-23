
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import "MacDwycoAudioOutput.h"

#include "dwvec.h"
#include <stdlib.h>

#include "audo.h"
#include "audmix.h"
#include "audth.h"

#include <stdio.h>
#include <string.h>

struct audpack
{
    char *buf;
    int len;
    int pos;
    int user_data;
};
//static DwVec<audpack> devq;
static DwVec<audpack> doneq;
static int local_audio_status;


// create an audio output device object. there is only
// ever one of these in the system at once (all the mixing of
// audio from different sources (net, zap playback, chat server, etc.)
// is handled by the internal mixer.
// do not acquire hardware devices in this function, do that in the
// "init" function below.
void
skel_audout_new(void *a)
{
    //fprintf(stderr, "audout new %p\n", a);
}

// turn off and release any devices you may have opened
void
skel_audout_delete(void *a)
{
    //fprintf(stderr, "audout del %p\n", a);
}

// init is called before audio output is to start streaming.
// you should program and acquire devices here. if you can't
// get a suitable output device, return 0. otherwise, return 1.
// you can start streaming silence here if needed.
// you may get multiple inits, if the device is already
// allocated and running, you should just return 1.
//
// the device should be prepared to output 16-bit signed little-endian
// data at 44.1khz (mono).
int
skel_audout_init(void *a)
{
    //fprintf(stderr, "audout init %p\n", a);
    local_audio_status = 1;
    return 1;
}


//
// this is where the data should be output to the device.
// buf points to the data (16-bit signed little-endian 44.1khz
// mono). the buffer is len bytes long. user_data is sent in
// by the caller, and should be regurgitated in the "done" routine
// below.
//
// generally, the dll is expecting a data flow like this:
// it calls device output repeatedly with some number of
// buffers it determines to be a good number that will allow
// the audio system to store and begin playing. the number of
// buffers is usually enough for about 250ms of audio (about 4
// buffers-worth). the dll expects the driver to begin playback as
// soon as it gets the data.
// the dll meanwhile will poll the driver's "device done" function
// in order to retrieve the buffer it sent in, which it then
// assumes the driver has released (the driver will no longer access it.)
//
// note that the buffer passed in is "owned" by the DLL, not the
// driver. the DLL will handle the memory management of the buffer.
// however, it guarantees it will not access the buffer memory
// until the driver releases it via the "done" call.
// generally, the best way to handle this is to use 2 queues:
// (1) a queue for buffers waiting to be handed to a hardware driver
// (2) a queue for buffers that have been played and are
// going to be returned to the DLL.
//
// you stick the buffer on q #1 (which could be a hardware driver q,
// but remember to make a private copy of the data to hand to the
// hardware driver).
// when that is complete, you put the buffer on q #2 (your "done"
// q). device_done will pick up the buffer from the q #2.
// when you put something on your "done q", you should poke the
// audio mixer (see below.)
//
// the DLL expects the buffers to be returned in the exact order
// it played them out.
//
// some of this is an artifact of the windows implementation of
// the audio drivers, since it appears that the windows drivers
// might actually be using the memory blocks as the target of
// DMA requests.
//
// another important thing: when a buffer is played out (or
// otherwise irrevocably committed to playback, like it has been
// queued in a hardware driver queue) you need to poke the
// audio mixer thread to update itself:
//
// if(TheAudioMixer)
//  TheAudioMixer->done_callback();
//
// there is no need to specify the exact buffer, the audio mixer
// has a record of what it has sent to the driver and knows what to do.
//
int
skel_audout_device_output(void *ah, void *buf, int len, int user_data)
{
    //fprintf(stderr, "MAC audout dev output %p buf %p len %d ud %d\n", ah, buf, len, user_data);
    // q up the data so that it can be pushed out in the callback
    // function.
    struct audpack a;
    a.buf = (char *)buf;
    a.len = len;
    a.user_data = user_data;
    a.pos = 0;

    MacDwycoAudioOutput * macOutput = [MacDwycoAudioOutput getSharedOutput];
    //NSLog(@"Using macOutput instance: %@", macOutput);
    [macOutput pushDwycoReceivedAudioData:a.buf length:a.len];
    if (![macOutput isQueueRunning])
        [macOutput startQueue];

    // Note, the macOutput object creates its own copy of the data and
    // the push method is multi-thread-safe.

    //SDL_LockAudio();
    //devq.append(a);
    //SDL_UnlockAudio();
    // ok, just pretend we sent it to the audio hardware q
    // <shazam> done.

    // now we q up the buffer as being "done" and poke the mixer
    doneq.append(a);
    mixer_buf_done_callback();
    return 1;
}

int
skel_audout_device_done(void *pa, void **buf_out, int *len, int *user_data)
{
    //fprintf(stderr, "audout dev done %p ", pa);
    //SDL_LockAudio();
    if(doneq.num_elems() > 0)
    {
        struct audpack a = doneq[0];
        *buf_out = a.buf;
        *len = a.len;
        *user_data = a.user_data;
        doneq.del(0);
        //fprintf(stderr, "ret %p %d %d (q len = %ld)\n", a.buf, a.len, a.user_data, doneq.num_elems());
        //SDL_UnlockAudio();
        return 1;
    }
    //fprintf(stderr, "nothing\n");
    //SDL_UnlockAudio();
    return 0;

}

// this should stop playback without
int
skel_audout_device_stop(void *a)
{
    //fprintf(stderr, "audout dev stop %p\n", a);

    MacDwycoAudioOutput * macOutput = [MacDwycoAudioOutput getSharedOutput];
    [macOutput pauseQueue];

    //SDL_PauseAudio(1);
    return 1;
}

// this should stop playback, and move any buffers that
// have not been returned to the DLL into your "done q" so
// that they can be returned.
int
skel_audout_device_reset(void *a)
{
    //fprintf(stderr, "audout dev reset %p doneq = %ld\n", a, doneq.num_elems());

    MacDwycoAudioOutput * macOutput = [MacDwycoAudioOutput getSharedOutput];
    [macOutput pauseQueue];
    [macOutput resetDwycoAudioData];

#if 0
    SDL_PauseAudio(1);
    SDL_LockAudio();
    int n = devq.num_elems();
    for(int i = 0; i < n; ++i)
    {
        doneq.append(devq[i]);
    }
    devq.set_size(0);
    SDL_UnlockAudio();
#endif
    return 1;
}

// return 1 if the device has been successfully initialized
//
int
skel_audout_device_status(void *a)
{
    //fprintf(stderr, "audout dev status %p %d\n", a, local_audio_status);
    return local_audio_status;
}

// reset the device and release it
int
skel_audout_device_close(void *ah)
{
    //fprintf(stderr, "audout dev close %p\n", ah);

    skel_audout_device_reset(ah);
    //SDL_CloseAudio();
    return 1;
}

// return the number of milliseconds represented by sz bytes.
// just return 80 for now, since it is all hardwired in other
// places.
int
skel_audout_device_buffer_time(void *ah, int sz)
{
    //fprintf(stderr, "audout dev buffer time %p\n", ah);
    return 80;
    oopanic("fixme");
}

// play a frame of silence, use this as-is
int
skel_audout_device_play_silence(void *ah)
{
    //fprintf(stderr, "audout dev play silence %p\n", ah);
    DWBYTE *sil = new DWBYTE[AUDBUF_LEN];
    memset(sil, 0, AUDBUF_LEN);
    skel_audout_device_output(ah, sil, AUDBUF_LEN, 1);
    return 1;
}

// this function returns the number of buffers that
// are currently queued up and playing out via the hardware.
// the DLL uses this to avoid playing too many buffers into the
// hardware (where they can't be fiddled with.) if the numbers
// drops below a threshold, it will also hurry up and feed the
// driver more buffers in order to avoid underruns.
//
int
skel_audout_device_bufs_playing(void *ah)
{
    MacDwycoAudioOutput * macOutput = [MacDwycoAudioOutput getSharedOutput];
    //NSLog(@"Using macOutput instance: %@", macOutput);
    int n = [macOutput getNumPlaying];
    //fprintf(stderr, "audout dev bufs playing %p, %d\n", ah, n);
    //return rand() % 4;
    return n;
}
