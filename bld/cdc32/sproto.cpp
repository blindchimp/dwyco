
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// simple protocol object
//
// each row is:
// state-name event action next-state (optional alternate next-state)
//
// where event is r, w, i (immediate), t (timeout) or "rw".
// "rw" means "read or write".
// action is a pointer to a member function in the channel.
// next state is the state that is entered if the action returns "next".
// likewise for alternate next state. this gives a simple way to branch in a protocol.
// "t" is a transition that is taken when the user-defined timer expires. the timer
// is normally not active. states can enable the timer, and must control it
// explicitly. NOTE: this is different than the default timer (see below.)
//
// all of these protocols are expected to make progress through their states
// at a reasonable pace. a default timer is set on protocol initiation that
// is extended each time a state handler is fired. if the protocol fails to
// fire any handlers for a set time, the protocol will be forced into the "fail" state.
// this watchdog timer can be disabled or modified if the protocol has entered
// a state that has different assumptions regarding how often i/o is performed.
// for example, once the media subchannel protocol is set up, it can sit idle
// except for the occasional (5 minutes) ping.
//
// the member function should be
// int (MMChannel::*)(int subchan, sproto *p, const char *events)
// return next for normal, transition to next state
// return alt_next for normal, transition to alt-state
// return stay to remain in current state
// return fail to stop the protocol and drop the entire channel (and
// all subchannels too.)
//
// special state names:
// "end" means stop the protocol and drop the subchannel (but not the channel)
//
// "stop" means stop the protocol but do not drop the subchannel
// this is only used when the subchannel needs to be handed off to
// another protocol based on something more complicated, e.g.
// the contents of incoming data determines the next protocol to use.
//

#include <string.h>
#include "sproto.h"
#include "dwstr.h"
#include "mmchan.h"
#include "dwrtlog.h"
#include "ta.h"

using namespace dwyco;

sproto::sproto(int sc, strans *tr, ValidPtr v)
    : mvp(v), timeout("sproto-timeout"), watchdog("sproto-watchdog")
{
    cur_state = 0;
    trans = tr;
    subchan = sc;
    running = 0;
}

void
sproto::start()
{
    running = 1;
    rearm_watchdog();
}

void
sproto::rearm_watchdog()
{
    watchdog.load(60 * 1000);
    watchdog.start();
}

void sproto::end()
{
    running = 0;
}

int
sproto::find(const char *state)
{
    int i = 0;
    while(trans[i].state)
    {
        if(strcmp(trans[i].state, state) == 0)
            return i;
        ++i;
    }
    return -1;
}

int
sproto::crank()
{
    if(!running)
        return 0;
    const char *ev = trans[cur_state].events;
    if(!mvp.is_valid())
    {
        return -1;
    }
    MMChannel *m = (MMChannel *)(void *)mvp;
    DwString evs;
    int ret = none;

    if(watchdog.is_expired())
    {
        watchdog.ack_expire();
        ret = fail;
        GRTLOGA("watchdog timeout %p", this, 0, 0, 0, 0);
        TRACK_ADD(SPROTO_watchdog_timeout, 1);
    }
    else
    {
        if(strchr(ev, 't'))
        {
            if(timeout.is_expired())
            {
                timeout.ack_expire();
                evs += "t";
                if(trans[cur_state].action)
                {
                    GRTLOGA("fire %p %d %s %s", this, subchan, trans[cur_state].state, evs.c_str(), 0);
                    ret = (m->*trans[cur_state].action)(subchan, this, evs.c_str());
                }
                else
                    ret = next;
            }
        }
        if(strchr(ev, 'i'))
        {
            evs += "i";
            GRTLOGA("fire %p %d %s %s", this, subchan, trans[cur_state].state, evs.c_str(), 0);
            ret = (m->*(trans[cur_state].action))(subchan, this, evs.c_str());
        }
        else
        {
            int fire = 0;
            if(strchr(ev, 'r'))
            {
                if(m->tube->has_data(subchan))
                {
                    evs += "r";
                    fire = 1;
                }

            }
            if(strchr(ev, 'w'))
            {
                if(m->tube->can_write_data(subchan))
                {
                    evs += "w";
                    fire = 1;
                }
            }
            if(fire)
            {
                GRTLOGA("fire %p %d %s %s", this, subchan, trans[cur_state].state, evs.c_str(), 0);
                ret = (m->*(trans[cur_state].action))(subchan, this, evs.c_str());
            }

        }
    }
    if(ret != none)
    {
        // NOTE: we assume if you are staying in the same state, you are not
        // making progress. if you are looping back to the same state (like in a
        // data transfer where you are sending chunks of data over and over)
        // AND you are making progress, then you have to rearm the watchdog
        // explicitly in your handler
        if(ret != stay)
        {
            if(watchdog.is_running())
            {
                rearm_watchdog();
                GRTLOG("rearm watchdog %p", this, 0);
            }
            else
            {
                GRTLOG("no rearm watchdog %p", this, 0);
            }
        }
        GRTLOGA("returns %p %d %d", this, subchan, ret, 0, 0);

    }
    if(ret == next || ret == alt_next)
    {
        // next state
        const char *ns = (ret == next ? trans[cur_state].next_state :
                          trans[cur_state].alt_next_state);
        if(ns == 0)
        {
            oopanic("bad state table");
        }
        if(strcmp(ns, "end") == 0)
        {
            running = 0;
            m->tube->drop_channel(subchan);
            return -2;
        }
        else if(strcmp(ns, "stop") == 0)
        {
            running = 0;
            return -2;
        }
        int i = find(ns);
        if(i == -1)
            oopanic("bad next");
        cur_state = i;
        return 1;
    }
    if(ret == stay)
    {
        return 1;
    }
    if(ret == fail)
    {
        running = 0;
        m->tube->drop_channel(subchan);
        m->schedule_destroy();
        return -1;
    }
    if(ret == none)
    {
        // no events, but no error either
        return 1;
    }
    oopanic("bogus handler return");
    return -1;
}


