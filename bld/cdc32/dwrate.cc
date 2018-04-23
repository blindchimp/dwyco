
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwrate.cc 1.18 1999/01/10 16:09:31 dwight Checkpoint $
 */
#include "dwrate.h"

DwRateMonitor::DwRateMonitor(long interval, long rd) : timer("rate")
{
    //timer.set_oneshot(0);
    timer.set_autoreload(1);
    timer.set_interval(interval);
    timer.reset();
    timer.start();
    rate_mult = rd;
    units = 0;
    last_units = 0;
    unit_rate = 0;
}

void
DwRateMonitor::add_units(long l)
{
    units += l;
}

void
DwRateMonitor::add_units_sliding(long l)
{
    unitbuf.append(l);
    unittime.append(timer.time_now());
}

long
DwRateMonitor::get_rate()
{
    return unit_rate;
}

long
DwRateMonitor::get_rate_sliding()
{
    int n = unitbuf.num_elems();
    long t = timer.time_now();
    double tot = 0;
    while(n >= 1 && t - unittime[n - 1] <= timer.get_interval())
    {
        tot += unitbuf[n - 1];
        --n;
    }

    unittime.del(0, n);
    unitbuf.del(0, n);

    if(timer.get_interval() == 0)
        return 0;
    return (long)((tot * rate_mult) / timer.get_interval());
}

int
DwRateMonitor::is_expired()
{
    //timer.tick();
    if(timer.is_expired())
    {
        timer.ack_expire();
        unsigned long acint = timer.get_actual_interval();
        long num_units = units - last_units;
        if(acint == 0)
            unit_rate = 0;
        else
            unit_rate = (num_units * rate_mult) / acint;
        last_units = units;
        return 1;
    }
    return 0;
}

void
DwRateMonitor::start()
{
    timer.start();
}

void
DwRateMonitor::reset()
{
    units = 0;
    last_units = 0;
    unit_rate = 0;
    timer.reset();
}

void
DwRateMonitor::stop()
{
    timer.stop();
}
