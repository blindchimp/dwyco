
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef DWTIMER_H
#define DWTIMER_H

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
    enum Type { ONESHOT = 0, REPEATING = 1 };

    DwTimer(const char *timer_id = 0);
    virtual ~DwTimer();

    void start(Type type, dwtime_t first_expire_interval,
               dwtime_t following_expire_interval = 0);
    void stop();

    int  is_expired();
    void ack_expire();

    int is_running();
    int is_repeating();
    dwtime_t get_interval();
    dwtime_t get_time_left();
    dwtime_t get_actual_interval();

    void set_interval(dwtime_t t);

    static dwtime_t time_now();
    static dwtime_t next_expire_time(DwString&);

private:
    DwTimer(const DwTimer&) = delete;
    DwTimer& operator=(const DwTimer&) = delete;
    struct dwyco::timer timer;
    int repeating;
    int enabled;
    int expired;

    dwtime_t interval;
    dwtime_t first_interval;
    dwtime_t actual_interval;

#ifdef DW_RTLOG
    static int Id;
    char lid[100];
#endif
};

#endif
