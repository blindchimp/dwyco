
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/snds.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef SNDS_H
#define SNDS_H

void init_snds();

extern char *Over_snd;
void play_new_zap_alert();
void play_online_alert();
void play_call_alert();
void play_incoming_zap_alert();

#endif
