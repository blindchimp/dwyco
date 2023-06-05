
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aq.cc 1.15 1999/01/10 16:09:23 dwight Checkpoint $
 */
#include "dwstr.h"
#include "aq.h"

#include "acqfile.h"
#include "aqkey.h"
#include "mmchan.h"
#include "ezset.h"
#include "aqext.h"

#include "dwrtlog.h"
using namespace dwyco;

#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
#include "aqext_android.h"
#endif

KeyboardAcquire *TheMsgAq;
int Chatbox_id = -1;

VidAcquire *TheAq;

int ExternalVideoAcquisition;

static int Bound_setting;
void
rb_tweaked(vc name, vc val)
{
    if(TheAq)
    {
        ExtAcquire *ea = dynamic_cast<ExtAcquire *>(TheAq);
        if(ea)
            ea->set_swap_rb((int)val);
    }
}

#ifndef DWYCO_NO_VIDEO_FROM_PPM
static
int
init_raw_files(int mbox, DwString& fail_reason)
{
    FileAcquire<pixel> *a = new FileAcquire<pixel>;
    a->set_fail_reason("unknown raw file init");

    if(!a->init((int)get_settings_value("raw_files/use_pattern") == 1 ?
                (const char *)get_settings_value("raw_files/raw_files_pattern") :
                (const char *)get_settings_value("raw_files/raw_files_list"),

                (int)get_settings_value("raw_files/use_pattern"),
                (int)get_settings_value("raw_files/preload") == 1))
    {
        if(mbox)
        {
            DwString m("Raw file startup failure: ");
            m += a->get_fail_reason();
            (*MMChannel::popup_message_box_callback)(0, m.c_str(), vcnil, vcnil);
        }
        fail_reason = a->get_fail_reason();
        delete a;
        TheAq = 0;
        return 0;
    }
    TheAq = a;
    return 1;
}
#endif

int
init_external_video(int mbox)
{
#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
    ExtAcquireAndroid *a = new ExtAcquireAndroid;
#else
    ExtAcquire *a = new ExtAcquire;
#endif

    if(!a->init((int)get_settings_value("rate/max_fps")))
    {
        if(mbox)
        {
            if(MMChannel::popup_message_box_callback)
                (*MMChannel::popup_message_box_callback)(0, "video capture init failure", vcnil, vcnil);
        }
        delete a;
        TheAq = 0;
        return 0;
    }
    TheAq = a;
    return 1;
}

int
initaq(int mbox, DwString& fail_reason)
{
    fail_reason = "unknown video acq init failure";
    if(TheAq)
        return 1;
    if(!Bound_setting)
    {
        bind_sql_setting("video_format/swap_rb", rb_tweaked);
        Bound_setting = 1;
    }
#ifndef DWYCO_NO_VIDEO_FROM_PPM
    if(get_settings_value("video_input/source") == vc("raw"))
    {
        return init_raw_files(mbox, fail_reason);
    }
    else
#endif
    {
        if(ExternalVideoAcquisition)
        {
            return init_external_video(mbox);
        }
    }
    return 0;
}

void
exitaq()
{
    if(TheAq)
    {
        GRTLOG("stop the aq", 0, 0);
        TheAq->stop();
        //sleep(2);
        GRTLOG("delete the aq", 0, 0);
        MMChannel::clean_video_sampler_refs(TheAq);
        delete TheAq;
        TheAq = 0;
    }
}

void
exit_msgaq()
{
    if(!TheMsgAq)
        return;
    MMChannel::clean_keyboard_sampler_refs(TheMsgAq);
    delete TheMsgAq;
    TheMsgAq = 0;
    Chatbox_id = -1;
}


