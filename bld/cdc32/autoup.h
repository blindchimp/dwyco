
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//---------------------------------------------------------------------------
#ifndef AUTOUP_H
#define AUTOUP_H
//---------------------------------------------------------------------------

class MMChannel;
#include "vc.h"
#include "pval.h"
#include "dlli.h"

//---------------------------------------------------------------------------
class TAutoUpdate
{
public:
    TAutoUpdate();

    int fetch_update(DwycoStatusCallback scb, void *scb_arg1,
                     DwycoAutoUpdateDownloadCallback dcb);
    int fetch_update_background();
    void abort_fetch();
    int run();

    MMChannel * xfer_channel;
    vc check_results;
    ValidPtr vp;
    vc patch;
    vc patch_filename;
    int compulsory;
    int done_background;
    DwycoStatusCallback user_status_callback;
    void *user_scb_arg1;
    DwycoAutoUpdateDownloadCallback user_eo_xfer;
};
#endif
