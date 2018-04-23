
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TL_H
#define TL_H
#include "dwvec.h"
#include "dwtimer.h"

class DwTimeLine
{
public:
    DwTimeLine();
    virtual ~DwTimeLine();

    void append(unsigned long time, void *ud1, int ud2);
    void start();
    int is_running();
    void stop();
    void reset();
    int tick();
    int get_top(unsigned long&, void*&, int&);
    int done();
    int set_speed(double x);
    double get_speed();

private:
    DwVec<unsigned long> times;
    DwVec<void *> ud1;
    DwVec<int> ud2;
    int running;
    int cur;
    unsigned long t0;
    double speed;
    unsigned long offset;
};

#endif
