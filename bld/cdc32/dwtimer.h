
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwtimer.h 1.19 1999/01/10 16:10:47 dwight Checkpoint $
 */
#ifndef DWTIMER_H
#define DWTIMER_H
// objects that act like hardware timers of various sorts

// defs:
// one-shot: timer is initially loaded and counts down to 0, sets "expired"
//		and stops. otherwise, timer runs continuously.
// auto-reload: when the timer hits 0 or less, the timer is automatically reloaded
//	with the auto-reload value (but not necessarily started).
//
#ifndef __LP64__
#ifdef _Windows
typedef unsigned long dwtime_t;
typedef long sdwtime_t;
#else
typedef unsigned long long dwtime_t;
typedef long long sdwtime_t;
#endif
#else
typedef unsigned long dwtime_t;
typedef long sdwtime_t;
#endif

#include "dwcls_timer.h"

class DwTimer
{
public:
    DwTimer(const char *timer_id = 0);
    virtual ~DwTimer();

    static dwtime_t next_expire_time();

    int is_expired();
    void ack_expire();
    void set_oneshot(int);
    void set_interval(dwtime_t);
    dwtime_t get_interval();
    dwtime_t get_actual_interval();
    dwtime_t get_time_left();
    void set_autoreload(int);
    void load(dwtime_t);
    void reset();
    void start();
    void stop();

    int is_running();

private:
    DwTimer(const DwTimer&);
    DwTimer& operator=(const DwTimer&);
    struct dwyco::timer timer;
    int oneshot;
    int auto_reload;
    int enabled;

    dwtime_t interval;
    dwtime_t actual_interval;

#ifdef DW_RTLOG
    static int Id;
    char lid[100];
#endif

public:
    static dwtime_t time_now(); // os dep absolute time

};

#endif
