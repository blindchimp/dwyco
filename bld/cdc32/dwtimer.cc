
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
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

    interval = 0;
    first_interval = 0;
    enabled = 0;
    expired = 0;
    repeating = 0;
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

void
DwTimer::start(Type type, dwtime_t first, dwtime_t following)
{
    stop();
    repeating = type;
    first_interval = first;
    interval = following;
    enabled = 1;
    expired = 0;
    timer_set(&timer, first);
}

void
DwTimer::stop()
{
    enabled = 0;
    expired = 0;
    timer.stop();
}

int
DwTimer::is_expired()
{
    if(!enabled)
    {
        expired = 0;
        return 0;
    }
    int tmp = timer_expired(&timer);
    if(tmp)
    {
        expired = 1;
        sdwtime_t timenow = time_now();
        actual_interval = timenow - timer.start;
#ifdef LONGLONG
        TIMERLOG("exp %lld aint %lld", timenow, timenow - timer.start);
#else
        TIMERLOG("exp %ld aint %ld", timenow, timenow - timer.start);
#endif
    }
    return expired;
}

void
DwTimer::ack_expire()
{
    expired = 0;
    if(repeating)
    {
        timer_reset(&timer);
        enabled = 1;
    }
    else
    {
        stop();
    }
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

dwtime_t
DwTimer::get_time_left()
{
    if(!enabled)
    {
#ifdef NDEBUG
        return 0;
#else
        return 0;
#endif
    }
    sdwtime_t t = timer_remaining(&timer);
    return (t < 0) ? 0 : t;
}

int
DwTimer::is_running()
{
    return enabled;
}

int
DwTimer::is_repeating()
{
    return repeating;
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
