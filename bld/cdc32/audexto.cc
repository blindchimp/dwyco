
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "audexto.h"

#include "dlli.h"

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
int AudioOutputExternal::already;

AudioOutputExternal::AudioOutputExternal()
{
    if(already)
        oopanic("AudioOutputExternal not singleton");
    already = 1;
    if(dwyco_audout_new)
        (*dwyco_audout_new)(this);
}

AudioOutputExternal::~AudioOutputExternal()
{
    already = 0;
    if(dwyco_audout_delete)
        (*dwyco_audout_delete)(this);
}

int
AudioOutputExternal::device_init()
{
    if(dwyco_audout_init)
        return (*dwyco_audout_init)(this);
    return 0;
}

int
AudioOutputExternal::device_output(DWBYTE *buf, int len, int user_data)
{
    if(dwyco_audout_output)
        return (*dwyco_audout_output)(this, buf, len, user_data);
    return 0;
}

int
AudioOutputExternal::device_done(DWBYTE*& buf, int& len, int& do_free)
{
    if(dwyco_audout_done)
        return (*dwyco_audout_done)(this, &(void *&)buf, &len, &do_free);
    return 0;
}

int
AudioOutputExternal::device_stop()
{
    if(dwyco_audout_stop)
        return (*dwyco_audout_stop)(this);
    return 0;
}

int
AudioOutputExternal::device_reset()
{
    if(dwyco_audout_reset)
        return (*dwyco_audout_reset)(this);
    return 0;
}

int
AudioOutputExternal::device_status()
{
    if(dwyco_audout_status)
        return (*dwyco_audout_status)(this);
    return 0;
}

int
AudioOutputExternal::device_close()
{
    if(dwyco_audout_close)
        return (*dwyco_audout_close)(this);
    return 0;
}

int
AudioOutputExternal::device_play_silence()
{
    if(dwyco_audout_play_silence)
        return (*dwyco_audout_play_silence)(this);
    return 0;
}

int
AudioOutputExternal::device_bufs_playing()
{
    if(dwyco_audout_bufs_playing)
        return (*dwyco_audout_bufs_playing)(this);
    return 0;
}

int
AudioOutputExternal::device_buffer_time(int sz)
{
    if(dwyco_audout_buffer_time)
        return (*dwyco_audout_buffer_time)(this, sz);
    return 80;
}

int
AudioOutputExternal::device_one_buffer_time()
{
    return device_buffer_time(1000 /*FIXME BROKEN*/);
}
