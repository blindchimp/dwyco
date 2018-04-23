
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dwvec.h"
#include <SDL/SDL.h>
#include "speex/speex_echo.h"
#include "audo.h"
#include "matcom.h"
#include "dwrtlog.h"
#include "audmix.h"
#include "audth.h"

#if 0
extern DwycoVVCallback dwyco_audout_new;
extern DwycoVVCallback dwyco_audout_delete;
extern DwycoIVCallback dwyco_audout_init;
extern DwycoDevOutputCallback dwyco_audout_output;
extern DwycoDevDoneCallback dwyco_audout_done;
extern DwycoIVCallback dwyco_audout_stop;
extern DwycoIVCallback dwyco_audout_reset;
extern DwycoIVCallback dwyco_audout_status;
extern DwycoIVCallback dwyco_audout_close;
extern DwycoIICallback dwyco_audout_buffer_time;
extern DwycoIVCallback dwyco_audout_play_silence;
extern DwycoIVCallback dwyco_audout_bufs_playing;
#endif

struct audpack
{
    char *buf;
    int len;
    int pos;
    int user_data;
};
static DwVec<audpack> devq;
static DwVec<audpack> doneq;
static int SDL_audio;
SpeexEchoState *Echo_state;

static
void
sdl_filler(void *userdata, Uint8 *buf, int len)
{
    GRTLOGA("filler devq %d doneq %d len %d", devq.num_elems(), doneq.num_elems(), len, 0, 0);
    if(devq.num_elems() == 0)
    {
        // just load it up with silence, there is a buffer underrun
        // happening
        memset(buf, 0, len);
#if 0
// hmmm, might need to do some locking or something here
        if(Echo_state)
            speex_echo_playback(Echo_state, (const spx_int16_t *)buf);
#endif
        return;
    }

    int toload = len;
    while(toload > 0 && devq.num_elems() > 0)
    {
        if(toload < devq[0].len - devq[0].pos)
        {
            memcpy(buf, devq[0].buf + devq[0].pos, toload);
            devq[0].pos += toload;
            toload = 0;
            buf += toload;
            break;
        }
        else if(toload >= devq[0].len - devq[0].pos)
        {
            memcpy(buf, devq[0].buf + devq[0].pos, devq[0].len - devq[0].pos);
            buf += devq[0].len - devq[0].pos;
            toload -= devq[0].len - devq[0].pos;
            doneq.append(devq[0]);
            devq.del(0);
            // poke the mixer to update itself, note this is different
            // than windows, where a buffer is "done" after it is played
            // out (i think, at least that is what is implied in the ms docs.)
            // in this case, we signal it is "done" when it goes out to the
            // driver, but we don't know when it is actually going to be played
            // in real time (because it may just be q'd up in a buffer).
            //if(TheAudioMixer)
            //TheAudioMixer->done_callback();
            mixer_buf_done_callback();
        }
    }
    if(toload > 0 && devq.num_elems() == 0)
    {
        // buffer underrun, just zero out the last
        // part to avoid bogus noise
        memset(buf, 0, len - toload);
    }
#if 0
    if(Echo_state)
        speex_echo_playback(Echo_state, (const spx_int16_t *)buf);
#endif
}

void
audout_sdl_new(void *)
{
    devq.set_size(0);
    doneq.set_size(0);
}

void
audout_sdl_delete(void *)
{
    SDL_CloseAudio();
    devq.set_size(0);
    doneq.set_size(0);
    SDL_audio = 0;
    if(Echo_state)
    {
        speex_echo_state_destroy(Echo_state);
        Echo_state = 0;
    }
}

int
audout_sdl_init(void *ha)
{
    if(SDL_audio)
        return 1;
    if(!SDL_WasInit(SDL_INIT_AUDIO))
    {
        if(SDL_Init(SDL_INIT_AUDIO) == -1)
            return 0;
    }
    SDL_AudioSpec sa;
    sa.freq = UWB_SAMPLE_RATE;
    sa.format = AUDIO_S16SYS;
    sa.channels = 1;
    //sa.samples = AUDBUF_LEN / 2;
    sa.samples = 2 * ECHO_SAMPLES;
    sa.callback = sdl_filler;
    sa.userdata = ha;

#ifdef ECHO_CANCEL
    Echo_state = speex_echo_state_init(ECHO_SAMPLES, ECHO_TAIL_SAMPLES);
    int tmp = UWB_SAMPLE_RATE;
    speex_echo_ctl(Echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &tmp);
#endif


    if(SDL_OpenAudio(&sa, 0) == -1)
    {
        SDL_audio = 0;
        return 0;
    }
    SDL_PauseAudio(0);
    SDL_audio = 1;
    return 1;
}


int
audout_sdl_device_output(void *, void *buf, int len, int user_data)
{
    GRTLOG("sdl dev out %d", len, 0);
    // q up the data so that it can be pushed out in the callback
    // function.
    struct audpack a;
    a.buf = (char *)buf;
    a.len = len;
    a.user_data = user_data;
    a.pos = 0;
    if(Echo_state)
    {
        int frames = (len / 2) / ECHO_SAMPLES;
        spx_int16_t *s = (spx_int16_t *)buf;
        for(int i = 0; i < frames; ++i)
        {
            speex_echo_playback(Echo_state, s);
            s += ECHO_SAMPLES;
        }
    }
    SDL_LockAudio();
    devq.append(a);
    SDL_UnlockAudio();
    return 1;
}

int
audout_sdl_device_done(void *, void **buf_out, int *len, int *user_data)
{
    SDL_LockAudio();
    if(doneq.num_elems() > 0)
    {
        struct audpack a = doneq[0];
        *buf_out = a.buf;
        *len = a.len;
        *user_data = a.user_data;
        doneq.del(0);
        SDL_UnlockAudio();
        return 1;
    }
    SDL_UnlockAudio();
    return 0;

}

int
audout_sdl_device_stop(void *)
{
    SDL_PauseAudio(1);
    return 1;
}

int
audout_sdl_device_reset(void *)
{
    SDL_PauseAudio(1);
    SDL_LockAudio();
    int n = devq.num_elems();
    for(int i = 0; i < n; ++i)
    {
        doneq.append(devq[i]);
    }
    devq.set_size(0);
    SDL_UnlockAudio();
    if(Echo_state)
        speex_echo_state_reset(Echo_state);
    return 1;
}

int
audout_sdl_device_status(void *)
{
    return SDL_audio;
}

int
audout_sdl_device_close(void *ah)
{
    audout_sdl_device_reset(ah);
    SDL_CloseAudio();
    if(Echo_state)
    {
        speex_echo_state_destroy(Echo_state);
        Echo_state = 0;
    }
    return 1;
}

int
audout_sdl_device_buffer_time(void *, int sz)
{
    return 80;
    oopanic("fixme");
}

int
audout_sdl_device_play_silence(void *ah)
{
    DWBYTE *sil = new DWBYTE[AUDBUF_LEN];
    memset(sil, 0, AUDBUF_LEN);
    audout_sdl_device_output(ah, sil, AUDBUF_LEN, 1);
    return 1;
}

int
audout_sdl_device_bufs_playing(void *ah)
{
    SDL_LockAudio();
    int tmp = devq.num_elems();
    SDL_UnlockAudio();
    //if(SDL_GetAudioStatus() == SDL_AUDIO_PLAYING)
    //		return 1;
    //return 0;
    return tmp;
}
