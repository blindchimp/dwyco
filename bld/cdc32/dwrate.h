
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwrate.h 1.2 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef DWRATE_H
#define DWRATE_H
#include "dwtimer.h"
#include "dwvec.h"
//
// all times are in milliseconds. by default, the
// rate is calculated as units/second.
//
class DwRateMonitor
{
public:
#ifdef DWYCO_RATE_DISPLAY
    DwRateMonitor(long update_interval, long rate_div = 1000L);
    ~DwRateMonitor() {}

    void add_units(long);
    void add_units_sliding(long);
    long get_rate_sliding();
    long get_rate();
    int is_expired();
    void reset();
    void start();
    void stop();

private:
    DwTimer timer;
    double units;
    double last_units;
    long unit_rate;
    long rate_mult;
    DwVec<long> unittime;
    DwVec<long> unitbuf;
#else
    DwRateMonitor(long update_interval, long rate_div = 1000L){}
    ~DwRateMonitor() {}

    void add_units(long){}
    void add_units_sliding(long){}
    long get_rate_sliding(){return 0;}
    long get_rate(){return 0;}
    int is_expired(){return 0;}
    void reset(){}
    void start(){}
    void stop(){}
#endif

};
#endif
