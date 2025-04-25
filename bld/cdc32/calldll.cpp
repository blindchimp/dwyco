
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

namespace dwyco {

MMCallDLL::MMCallDLL(vc auid, vc ahost, vc aport, vc aproxinfo, int amsg_chan,
                     int auser_control_chan,
                     DwycoCallDispositionCallback cdc, void *cdc_arg1,
                     DwycoStatusCallback ascb, void *ascb_arg1,
                     CallStatusCallback bscb, void *bscb_arg1, ValidPtr bscb_arg2):
    MMCall(auid, ahost, aport, aproxinfo, amsg_chan, auser_control_chan, bscb, bscb_arg1, bscb_arg2)
{
    dscb = ascb;
    dscb_arg1 = ascb_arg1;
    //dcb = 0;
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

}
