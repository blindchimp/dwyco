
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SPROTO_H
#define SPROTO_H

#include "pval.h"
#include "dwtimer.h"
#include "vc.h"
#include "dwstr.h"

class MMChannel;
class sproto;

struct strans
{
    const char *state;
    const char *events;
    int (MMChannel::*action)(int subchan, sproto *, const char *events);
    const char *next_state;
    const char *alt_next_state;
};

class sproto
{
    friend class MMChannel;
    sproto(const sproto&) = delete;
    sproto& operator=(const sproto&) = delete;

public:
    // the ValidPtr in this case is a backpointer to the
    // MMChannel this sproto is a subchannel of
    sproto(int sc, strans *tr, ValidPtr v);
    void start();

private:
    enum handler_ret {
        none,
        next,
        alt_next,
        stay,
        fail
    };

    ValidPtr mvp; // backpointer to MMChannel
    int cur_state;
    int running;
    strans *trans;
    int subchan;
    // user controlled timeout
    DwTimer timeout;
    // watchdog timer to catch stalled protocols
    DwTimer watchdog;
    void rearm_watchdog();
    vc user_info;
    DwString fn_to_send;
    DwString recv_fn;
    int crank();
    int find(const char *state);
    void end();

};


// this one is for protocol that allows restart on sends
#define DWYCO_SEND_FILE_PORT_OFFSET 6


#endif
