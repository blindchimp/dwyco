
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CALLDLL_H
#define CALLDLL_H

#include "mmcall.h"
#include "dlli.h"

class MMCallDLL : public MMCall
{
public:

    MMCallDLL(vc uid, vc host, vc port, vc proxinfo, int msg_chan,
              int user_control_chan,
              DwycoCallDispositionCallback cdc, void *cdc_arg1,
              DwycoStatusCallback ascb, void *ascb_arg1,
              CallStatusCallback scb, void *scb_arg1, ValidPtr scb_arg2);

    DwycoStatusCallback dscb;
    void *dscb_arg1;
    DwycoChannelDestroyCallback dcb;
    void *dcb_arg1;
    DwycoCallDispositionCallback cdc;
    void *cdc_arg1;

    MessageDisplay *get_ui_msg_output();
};
#endif
