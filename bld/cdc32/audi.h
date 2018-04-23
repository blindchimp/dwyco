
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audi.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef AUDI_H
#define AUDI_H
class AudioAcquire;

int init_audio_input(int locally_invoked, int force);
int exit_audio_input();

extern AudioAcquire *TheAudioInput;
extern int ExternAudioAcquisition;

#endif
