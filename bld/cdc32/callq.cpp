
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// a call q is a set of calls that are in the process of being
// set up. the reason we need this is because we may be in a case
// where we need to set up a whole pile of calls, but don't necessarily
// want to set them up all at once.
// this occurs if we enter a room, and click "enter", and there
// are 50 or 60 people in there, we would have to have 50 or 60
// places on the screen set up to watch them all.
// usually people want the first 8 or so, then as those are
// rejected or disconnected, we can pick the next one in the q and
// start that call.
//
#include "callq.h"
#include "mmcall.h"
#include "dwrtlog.h"
#include "qauth.h"

using namespace dwyco;
extern int Media_select;

#define CQ_WAITING 0
#define CQ_CONNECTED 1
#define CQ_CONNECTING 2
#define CQ_FAILED 3
#define CQ_TERMINATED 4
#define CALLQ_POLL_TIME (10000)
#define CALLQ_SLOW_POLL_TIME (60000)

namespace dwyco {
struct callq
{
    // holds the MMCall pointer we are tracking
    // which is why we don't invalidate it here
    ValidPtr vp;
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

CallQ *TheCallQ;

void
init_callq()
{
    if(!TheCallQ)
        TheCallQ = new CallQ;
}

// note: this isn't useful except for
// leak tracing. generally, it is better to
// just cancel existing calls.
void
exit_callq()
{
    delete TheCallQ;
    TheCallQ = nullptr;
}

void
callq_tick()
{
    if(TheCallQ)
        TheCallQ->tick();
}

CallQ::CallQ() : call_q_timer("callq")
{
    max_established = 4;
    call_q_timer.load(CALLQ_POLL_TIME);
    //call_q_timer.set_oneshot(0);
    call_q_timer.set_autoreload(1);
    call_q_timer.set_interval(CALLQ_POLL_TIME);
    call_q_timer.reset();
    call_q_timer.start();
}

CallQ::~CallQ()
{
#if 0
    // note: these objects reference mmcall
    // objects, but only weakly.
    // at the point this dtor is used, the system
    // is shutting down, so worrying about graceful
    // cancellation is probably not useful.
    for(int i = 0; i < calls.num_elems(); ++i)
    {
        delete calls[i];
    }
#endif
}

void
CallQ::reset_poll_time(dwtime_t interval)
{
    call_q_timer.stop();
    call_q_timer.set_interval(interval);
    call_q_timer.start();
}

void
CallQ::set_max_established(int n)
{
    max_established = n;
}


void
CallQ::cq_call_status(MMCall *mmc, int status, void *arg1, ValidPtr vp)
{
    if(!TheCallQ || !vp.is_valid())
        return;
    int i;
    DwVecP<struct callq> &cq = TheCallQ->calls;
    int n = cq.num_elems();
    for(i = 0; i < n; ++i)
    {
        if(cq[i] && vp == cq[i]->vp)
            break;
    }
    if(i == n)
        return;
    GRTLOG("callq status %d: %d", vp.cookie, status);
    if(status == MMCALL_ESTABLISHED)
        cq[i]->status = CQ_CONNECTED;
    else if(status == MMCALL_TERMINATED)
        cq[i]->status = CQ_TERMINATED;
    else
        cq[i]->status = CQ_FAILED;
    if(cq[i]->user_cb)
    {
        (*(cq[i]->user_cb))(mmc, status, cq[i]->arg1, cq[i]->arg2);
    }
    TheCallQ->reset_poll_time(CALLQ_POLL_TIME);
}

int
CallQ::add_call(MMCall *mmc)
{
    int i;
    callq *cq;
    // don't allow a call to the same uid to be q'd twice
    for(i = 0; i < calls.num_elems(); ++i)
    {
        if(calls[i] && calls[i]->vp.is_valid())
            if(mmc->uid == ((MMCall *)(void *)(calls[i]->vp))->uid)
                return 0;
    }
    if((i = calls.index(0)) == -1)
    {
        calls.append(cq = new callq(mmc->vp));
        i = calls.num_elems() - 1;
    }
    else
    {
        calls[i] = cq = new callq(mmc->vp);
    }
    GRTLOG("add_call %s id %d", (const char *)to_hex(mmc->uid), calls[i]->vp.cookie);
    // interpose our callback
    cq->user_cb = mmc->scb;
    cq->arg1 = mmc->scb_arg1;
    cq->arg2 = mmc->scb_arg2;
    mmc->scb = cq_call_status;
    mmc->scb_arg2 = mmc->vp;
    TheCallQ->reset_poll_time(CALLQ_POLL_TIME);
    return calls[i]->vp.cookie;
}

//int
//CallQ::cancel_call(int n)
//{
//    oopanic("this doesn't work");
//    return 0;
//}

int
CallQ::cancel_all()
{
    int n = calls.num_elems();
    for(int i = 0; i < n; ++i)
    {
        if(calls[i])
        {
            calls[i]->cancel = 1;
            if(calls[i]->vp.is_valid())
            {
                MMCall *mmc = (MMCall *)(void *)calls[i]->vp;
                mmc->cancel_call();
            }
        }
    }
    return 0;
}

// this function tries to figure out when to start
// up some more calls. there is some thinking to do
// regarding how you can figure out the total number
// of calls in progress at any given time. for the moment,
// we only count calls that are originated by the callq, tho
// it would probably make more sense to count all calls
// currently started.
int
CallQ::tick()
{
    static int been_here;

    if(!been_here)
    {
        call_q_timer.load(CALLQ_POLL_TIME);
        //call_q_timer.set_oneshot(0);
        call_q_timer.set_autoreload(1);
        call_q_timer.set_interval(CALLQ_POLL_TIME);
        call_q_timer.reset();
        call_q_timer.start();
        been_here = 1;
    }
    if(!call_q_timer.is_expired())
        return 0;
    call_q_timer.ack_expire();
    // here is where we cleanup dead calls
    // and figure out where we need to start up
    // new calls.
    int n = calls.num_elems();
    int connected = 0;
    int connecting = 0;
    int waiting = 0;
    int i;
    for(i = 0; i < n; ++i)
    {
        if(calls[i])
        {
            if(!calls[i]->vp.is_valid())
            {
                delete calls[i];
                calls[i] = 0;
            }
            else
            {
                if(calls[i]->status == CQ_CONNECTED)
                    ++connected;
                else if(calls[i]->status == CQ_CONNECTING)
                    ++connecting;
                else if(calls[i]->status == CQ_FAILED ||
                        calls[i]->status == CQ_TERMINATED)
                {
                    delete calls[i];
                    calls[i] = 0;
                }
                else if(calls[i]->cancel)
                {
                    GRTLOG("unstarted call dropped id: %d", calls[i]->vp.cookie, 0);
                    delete calls[i];
                    calls[i] = 0;
                }
                else
                    ++waiting;
            }
        }
    }
    if(connected >= max_established)
        return 0;
    if(connected + connecting >= max_established)
        return 0; // assume currently connecting things may work eventually
    // hack for windows and routers that don't allow a lot of
    // concurrent connection setups.
    if(connecting >= 3)
        return 0;

    if(connected + connecting + waiting == 0)
    {
        // note: there may be connected calls sitting around for
        // quite awhile, but they don't need any attention from the
        // callq except when they are finally terminated.
        // so reduce the polling interval to save some cycles
        reset_poll_time(CALLQ_SLOW_POLL_TIME);
        return 0;
    }

    // find calls that are waiting to be started and get
    // enough of them going.

    // MAY WANT TO ROUND ROBIN THIS WITH A STATIC
    if(waiting == 0)
        return 0;
    for(i = 0; i < n; ++i)
    {
        if(!calls[i])
            continue;
        if(calls[i]->status == CQ_WAITING && !calls[i]->cancel)
        {
            if(((MMCall *)(void *)calls[i]->vp)->start_call(Media_select))
            {
                calls[i]->status = CQ_CONNECTING;
                ++connecting;
            }
            if(connected + connecting >= max_established)
                break;
            // hack for windows and routers that don't allow a lot of
            // concurrent connection setups.
            if(connecting >= 3)
                break;
        }
    }
    return 0;
}
}
