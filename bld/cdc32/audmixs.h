
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audmixs.h 1.13 1999/01/10 16:10:57 dwight Checkpoint $
 */
#ifndef AUDMIXS_H
#define AUDMIXS_H
#include <windows.h>
#ifdef LINUX
#include "audexto.h"
#else
#include "audwin.h"
#endif

class AudioMixSend :
#ifdef LINUX
    public AudioOutput
#else
    public AudioOutputWin32PCM16
#endif
{
public:
    AudioMixSend(AudioMixer *m, int bufs, int bufsz);
    ~AudioMixSend();

    virtual int device_init();
    virtual int device_output(DWBYTE *buf, int len, int user_data);
    virtual int device_done(DWBYTE*& buf, int& len, int& do_free) ;
    virtual int device_stop();
    virtual int device_reset();
    virtual int device_close();
    virtual int device_status();
    virtual int device_done2(DWBYTE*& outbuf, int& len);
    virtual int device_bufs_playing();
    virtual int device_buffer_time(int len);
    virtual int device_one_buffer_time();
    virtual int device_play_silence();

private:
    static int Id;
    int myid;
    DWBYTE *buf_leftover;
    int len_leftover;

protected:
    virtual void callback(HWAVEOUT hwo, UINT msg, DWORD_PTR p1, DWORD_PTR p2);

};

#endif
