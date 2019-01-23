
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwtimer.cc 1.17 1999/01/10 16:09:32 dwight Checkpoint $
 */
#ifdef _Windows
#include <windows.h>
#endif

#include "dwtimer.h"

#include "dwrtlog.h"
#define TIMERLOG(fmt, a, b) GRTLOGA("%s " fmt, lid, (a), (b), 0, 0)
#ifndef __LP64__
#ifndef _Windows
#define LONGLONG
#endif
#endif

using namespace dwyco;

#ifdef DW_RTLOG
int DwTimer::Id;
#endif

DwTimer::DwTimer(const char *timer_id) :
    timer(timer_id)
{
#ifdef DW_RTLOG
    if(timer_id == 0)
    {
        sprintf(lid, "timer %d ", Id++);
    }
    else
    {
        sprintf(lid, "%s %d ", timer_id, Id++);
    }
#endif

    oneshot = 1;
    interval = 0;
    enabled = 0;
    auto_reload = 0;
    timer.interval = 0;
    timer.start = 0;
}

DwTimer::~DwTimer()
{
}

dwtime_t
DwTimer::next_expire_time(DwString& dbgstr)
{
    return timer::timer_next_expire(dbgstr);
}

// make is_expired return 0, and if it is autoreload, reset
// the timer and reenable so it can expire again
void
DwTimer::ack_expire()
{
    enabled = 0;
    timer.stop();
    if(auto_reload)
    {
        timer_reset(&timer);
        enabled = 1;
    }
}

int
DwTimer::is_expired()
{
    if(!enabled)
        return 0;
    int tmp = timer_expired(&timer);
    if(tmp)
    {
        sdwtime_t timenow = time_now();
        actual_interval = timenow - timer.start;
#ifdef LONGLONG
        TIMERLOG("exp %lld aint %lld", timenow, timenow - timer.start);
#else
        TIMERLOG("exp %ld aint %ld", timenow, timenow - timer.start);
#endif
    }
    return tmp;
}

void
DwTimer::set_interval(dwtime_t t)
{
    interval = t;
}

dwtime_t
DwTimer::get_interval()
{
    return interval;
}

dwtime_t
DwTimer::get_actual_interval()
{
    return actual_interval;
}

void
DwTimer::reset()
{
    enabled = 0;
    timer.stop();
}


void
DwTimer::set_oneshot(int o)
{
    oneshot = !!o;
    if(oneshot)
        auto_reload = 0;
}

void
DwTimer::load(dwtime_t t)
{
    interval = t;
}

dwtime_t
DwTimer::get_time_left()
{
    sdwtime_t t = timer_remaining(&timer);
    return (t < 0) ? 0 : t;
}

void
DwTimer::start()
{
    enabled = 1;
    timer_set(&timer, interval);
}

void
DwTimer::stop()
{
    enabled = 0;
    timer.stop();
}

void
DwTimer::set_autoreload(int a)
{
    auto_reload = !!a;
    if(auto_reload)
        oneshot = 0;
}

int
DwTimer::is_running()
{
    return enabled;
}

#ifdef LINUX
#ifdef MACOSX
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <unistd.h>
#endif

dwtime_t
DwTimer::time_now()
{
#ifdef _Windows
    return timeGetTime();
#elif defined(LINUX)
#ifdef MACOSX
    struct timeval tv;
    gettimeofday(&tv, 0);
    dwtime_t d = ((dwtime_t)tv.tv_sec * 1000 + tv.tv_usec / 1000);
    return d;
#else
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    dwtime_t d = ((dwtime_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    return d;
#endif
#else
#error fix timer routines for this os
#endif
}
