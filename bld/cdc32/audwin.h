
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef _Windows
/*
 * $Header: g:/dwight/repo/cdc32/rcs/audwin.h 1.18 1999/01/10 16:10:43 dwight Checkpoint $
 */
#ifndef AUDWIN_H
#define AUDWIN_H
#include <windows.h>
#include <mmsystem.h>
#include "dwvecp.h"
#include "audout.h"

class AudioMixer;

class AudioOutputWin32 : public AudioOutput
{
public:
    AudioOutputWin32(CRITICAL_SECTION& c, int decom = 0, int bufs = DEFAULT_AUDIO_BUFS);
    ~AudioOutputWin32();

    virtual int device_init();
    virtual int device_output(DWBYTE *buf, int len, int user_data);
    virtual int device_done(DWBYTE*& buf, int& len, int& do_free) ;
    virtual int device_stop();
    virtual int device_reset();
    virtual int device_close();
    virtual int device_status();
    virtual int device_play_silence();
    virtual int device_bufs_playing();

    void set_mixer(AudioMixer *);

    void set_aec(int);

protected:
    virtual int init_format(WAVEFORMATEX&) = 0;
    DWBYTE *silence;
    int silence_len;
    long num_out;
    virtual void callback(HWAVEOUT hwo, UINT msg, DWORD p1, DWORD p2);
    AudioMixer *mixer;

private:
    HWAVEOUT waveout;
    DwVecP<WAVEHDR> wavehdrs;
    CRITICAL_SECTION& cs;
    int aec;

    friend void CALLBACK cb_dispatch(HWAVEOUT hwo, UINT msg, DWORD obj, DWORD p1, DWORD p2);
};


class AudioOutputWin32PCM16 : public AudioOutputWin32
{
public:
    AudioOutputWin32PCM16(CRITICAL_SECTION& c, int decom = 0, int bufs = DEFAULT_AUDIO_BUFS, int bufsz = 0);
protected:
    int init_format(WAVEFORMATEX&);
    virtual int device_buffer_time(int sz);
    virtual int device_one_buffer_time();

};

#endif
#endif
