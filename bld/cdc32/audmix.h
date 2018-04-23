
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AUDMIX_H
#define AUDMIX_H
// $Header: g:/dwight/repo/cdc32/rcs/audmix.h 1.13 1999/01/10 16:10:58 dwight Checkpoint $
#include <windows.h>
#include "matcom.h"
#include "bcastq.h"
#include "dwvec.h"
#include "dwvecp.h"
class AudioOutput;
class AudioAcquire;


class AudioMixer
{
public:
    AudioMixer(int ringsize, CRITICAL_SECTION& c);
    virtual ~AudioMixer();

    void audio_mixer_thread();
    void set_driver(AudioOutput *);
    void set_audinp(AudioAcquire *);
    int device_bufs_playing(int id);
    void add(DWBYTE *buf, int len, int id);
    int device_one_buffer_time();
    int device_buffer_time(int len);

    int exit;

private:
    DwVecP<DWBYTE> ring;
    DwVec<int> ringlen;

    DwVec<int> srcs;
    DwVec<int> srcs_idx;
    DwVec<DwVec<int> > buf_contrib;
    DwVec<int> num_out;

    BroadcastQ inputq;

    CRITICAL_SECTION& cs;

    int current;
    int driver_current;
    int num_in_ring;
    HANDLE buf_done_event;
    AudioOutput *driver;
    AudioAcquire *audinp;

    friend class AudioOutputWin32;
    void release_bufs();
    void feed_driver();
    int unload_q();

public:
    // do yourself a favor and don't call this unless you are
    // absolutely sure you know what you are doing. main reason
    // this is public is because i can't be bothered with all the
    // friend decls
    void done_callback();

};

#endif
