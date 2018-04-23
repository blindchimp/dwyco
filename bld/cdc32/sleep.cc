
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/sleep.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "sleep.h"
#include "dwtimer.h"
#if defined(_Windows) || defined(__WIN32__)
extern int Sleeping;
void
dwsleep(int secs, int *force_ret, int pump)
{
    void pump_messages();
    DwTimer t;
    t.set_oneshot(1);
    t.load(secs * 1000L);
    t.start();

    Sleeping = 1;
    while(!t.is_expired())
    {
        if(force_ret && *force_ret)
            goto out;
        if(1 || pump)
            pump_messages();
    }
out:
    Sleeping = 0;
}
#endif
