
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "tl.h"
#include "dwrtlog.h"

// simple timeline
// you give it a bunch of numbers
// that are millisecond times and
// it returns true when the system clock
// equals or surpasses that value, then
// removes that value and waits for the
// next event. uses polling

DwTimeLine::DwTimeLine()
    : ud1(0, !DWVEC_FIXED, DWVEC_AUTO_EXPAND, 500),
      times(0, !DWVEC_FIXED, DWVEC_AUTO_EXPAND, 500),
      ud2(0, !DWVEC_FIXED, DWVEC_AUTO_EXPAND, 500)
{
    cur = 0;
    running = 0;
    speed = 1;
    offset = 0;
}

DwTimeLine::~DwTimeLine()
{
}

void
DwTimeLine::append(unsigned long time, void *user_data1, int user_data2)
{
    times.append(time);
    ud1.append(user_data1);
    ud2.append(user_data2);
}

void
DwTimeLine::reset()
{
    t0 = DwTimer::time_now();
    cur = 0;
    running = 0;
}

int
DwTimeLine::set_speed(double x)
{
    speed = x;
    stop();
    start();
    if(times.num_elems() == 0)
        return 1;
    if(cur >= times.num_elems())
        offset = times[times.num_elems() - 1];
    else
        offset = times[cur];
    return 1;
}

double
DwTimeLine::get_speed()
{
    return speed;
}

void
DwTimeLine::start()
{
    t0 = DwTimer::time_now();
    running = 1;
}

int
DwTimeLine::is_running()
{
    return running;
}

void
DwTimeLine::stop()
{
    running = 0;
}

int
DwTimeLine::done()
{
    return cur >= times.num_elems();
}

int
DwTimeLine::tick()
{
    if(!running || cur >= times.num_elems() || speed == 0)
        return 0;

    unsigned long tn = DwTimer::time_now();
    long tlv = tn - t0;
    //tlv *= speed;
    if(tlv >= (times[cur] - offset) / speed)
    {
        GRTLOG("tl ready at %d, late %d", times[cur], (int)(tlv - times[cur]));
        return 1;
    }
    return 0;

}

int
DwTimeLine::get_top(unsigned long& time_out, void*& ud1_out, int& ud2_out)
{
    if(cur >= times.num_elems())
        return 0;
    time_out = times[cur];
    ud1_out = ud1[cur];
    ud2_out = ud2[cur];
    ++cur;
    return 1;
}



