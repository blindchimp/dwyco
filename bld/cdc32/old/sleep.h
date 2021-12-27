
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/sleep.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef SLEEP_H
#define SLEEP_H

void dwsleep(int time, int *force_ret = 0, int pump = 0);
extern int Sleeping;

#endif
