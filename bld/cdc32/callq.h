
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CALLQ_H
#define CALLQ_H
#include "pval.h"
#include "dwtimer.h"
#include "mmcall.h"
#include "dwvecp.h"

struct callq
{
    ValidPtr vp;	// holds the MMCall pointer we are tracking
    DwTimer timeout;
    int status;
    // we interpose our stuff so we can track the call status
    CallStatusCallback user_cb;
    void *arg1;
    ValidPtr arg2;
    int cancel;

    callq(ValidPtr p) : vp(p) {
        status = 0;
        user_cb = 0;
        arg1 = 0;
        cancel = 0;
    }
};

class CallQ
{
public:
    CallQ();
    virtual ~CallQ();

    //int add_call(vc uid, vc host, vc port, vc proxinfo);
    int add_call(MMCall*);
    int cancel_call(int);
    int cancel_all();
    void set_max_established(int);
    int tick();

    DwVecP<struct callq> calls;
    int max_established;

};

// it is conceivable that you might want
// multiple call q's, but not now.
extern CallQ *TheCallQ;
void init_callq();
void callq_tick();

#endif
