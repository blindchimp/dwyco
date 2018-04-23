
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AUDTH_H
#define AUDTH_H

// $Header: g:/dwight/repo/cdc32/rcs/audth.h 1.11 1999/01/10 16:10:58 dwight Checkpoint $


int init_audio_mixer();
int end_mixer_thread();
int exit_audio_mixer();

// called by driver threads when it is done playing a buffer
void mixer_buf_done_callback();

class AudioMixer;
extern AudioMixer *volatile TheAudioMixer;

#endif
