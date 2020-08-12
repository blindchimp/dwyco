
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <windows.h>
#include "vc.h"

RECT ScreenSize;
vc Transmit_stats;
int Crashed_last_time;
vc StackDump;
int Sleeping;

unsigned int dwyco_rand_state;
