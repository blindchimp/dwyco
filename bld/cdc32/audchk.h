
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audchk.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef AUDCHK_H
#define AUDCHK_H

int check_audio_device();

extern int Audio_full_duplex;
extern int Audio_hw_full_duplex;
extern int Has_audio_output;
extern int Has_audio_input;
#endif
