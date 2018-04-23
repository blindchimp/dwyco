
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audo.h 1.18 1999/01/10 16:10:41 dwight Checkpoint $
 */
#ifndef AUDO_H
#define AUDO_H
class AudioOutput;

int init_audio_output(int);
int exit_audio_output();

extern AudioOutput *TheAudioOutput;

#define AUDBUF_LEN_8K 1280 /* in bytes , 80ms at 8khz */
#define AUDBUF_LEN_32K (AUDBUF_LEN_8K * 4)
#define AUDBUF_LEN_44_1K (7056)
#define AUDBUF_LEN_UWB ((int)(UWB_SAMPLE_RATE * .080 * 2))
#ifdef UWB_SAMPLING
#define AUDBUF_LEN AUDBUF_LEN_UWB
#else
#define AUDBUF_LEN AUDBUF_LEN_8K
#endif
#undef ECHO_CANCEL
#define ECHO_SAMPLES ((int)(UWB_SAMPLE_RATE * .020))
#define ECHO_TAIL_SAMPLES ((int)(UWB_SAMPLE_RATE * .100))

extern int Audbuf_len;
extern int ExternalAudioOutput;

#endif
