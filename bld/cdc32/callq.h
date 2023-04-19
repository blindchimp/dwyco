
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CALLQ_H
#define CALLQ_H
#include "mmcall.h"
#include "dwvecp.h"

namespace dwyco {

struct callq;

class CallQ
{
public:
    CallQ();
    ~CallQ();

    //int add_call(vc uid, vc host, vc port, vc proxinfo);
    int add_call(MMCall*);
    //int cancel_call(int);
    int cancel_all();
    void set_max_established(int);
    int get_max_established() {return max_established;}
    int tick();

private:

    DwVecP<struct callq> calls;
    int max_established;
    DwTimer call_q_timer;
    void reset_poll_time(dwtime_t);
    static void cq_call_status(MMCall *mmc, int status, void *arg1, ValidPtr vp);
};

// it is conceivable that you might want
// multiple call q's, but not now.
extern CallQ *TheCallQ;
void init_callq();
void exit_callq();
void callq_tick();

}

#endif
