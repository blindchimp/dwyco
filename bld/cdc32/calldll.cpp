
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "calldll.h"
#include "msgddll.h"
#include "dlli.h"

MMCallDLL::MMCallDLL(vc uid, vc host, vc port, vc proxinfo, int msg_chan,
                     int user_control_chan,
                     DwycoCallDispositionCallback cdc, void *cdc_arg1,
                     DwycoStatusCallback ascb, void *ascb_arg1,
                     CallStatusCallback scb, void *scb_arg1, ValidPtr scb_arg2):
    MMCall(uid, host, port, proxinfo, msg_chan, user_control_chan, scb, scb_arg1, scb_arg2)
{
    dscb = ascb;
    dscb_arg1 = ascb_arg1;
    dcb = 0;
    this->cdc = cdc;
    this->cdc_arg1 = cdc_arg1;
}

MessageDisplay *
MMCallDLL::get_ui_msg_output()
{
    // channel id doesn't make any sense any more because
    // multiple channels can be used to set up a call.
    // since it has to be unique, we just send out the
    // MMCall vp id, we'll have to change a bunch of stuff
    // to use the right id instead of channel id's
    return new MsgDisplayDLL(dscb, vp.cookie, dscb_arg1);
}
