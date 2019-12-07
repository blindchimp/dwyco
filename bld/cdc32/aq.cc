
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
#include "gvchild.h"
#include "dwstr.h"

#include "vidinput.h"
#include "ratetwkr.h"
#include "vfwinvst.h"
#include "aq.h"

#include "aqvfw.h"
#include "rawfiles.h"
#include "acqfile.h"
#include "vfwinvst.h"

#include "aqkey.h"

#include "mmchan.h"
#include "chatgrid.h"

#include "dwrtlog.h"
#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
#include "aqext_android.h"
#endif

class VFWShit;
extern VFWShit *TheVFWMgr;

KeyboardAcquire *TheMsgAq;
int Chatbox_id = -1;

int Has_VFW;
VidAcquire *TheAq;
enum aq_style Aq_style;

int ExternalVideoAcquisition;
void vspin();
void vunlock();

void
initvfw()
{
#ifndef USE_VFW
    Has_VFW = 0;
    return;
#else
    char szDeviceName[80];
    char szDeviceVersion[80];

    for (int wIndex = 0; wIndex < 10; wIndex++) {
        if (capGetDriverDescription (wIndex, szDeviceName,
                                     sizeof (szDeviceName), szDeviceVersion,
                                     sizeof (szDeviceVersion)))
        {
            ++Has_VFW;
            break;
        }
    }
    return;
#endif
}

#ifndef DWYCO_NO_VIDEO_FROM_PPM
static
int
init_raw_files(int mbox, DwString& fail_reason)
{
    FileAcquire<pixel> *a = new FileAcquire<pixel>;
    a->set_fail_reason("unknown");

    if(!a->init(RawFilesData.get_use_pattern() ? RawFilesData.get_raw_files_pattern() : RawFilesData.get_raw_files_list(),
                RawFilesData.get_use_pattern() ? 1 : 0, RawFilesData.get_preload()))
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

#include "aqext.h"

int
init_external_video(int mbox)
{
#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
    ExtAcquireAndroid *a = new ExtAcquireAndroid;
#else
    ExtAcquire *a = new ExtAcquire;
#endif

    if(!a->init(RTUserDefaults.get_max_frame_rate()))
    {
        if(mbox)
        {
            char m[255];
            sprintf(m, "video capture init failure");
            (*MMChannel::popup_message_box_callback)(0, m, vcnil, vcnil);
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
    fail_reason = "unknown";
    if(TheAq)
        return 1;
#ifndef DWYCO_NO_VIDEO_FROM_PPM
    if(VidInputData.get_raw())
    {
        return init_raw_files(mbox, fail_reason);
    }
    else
#endif
            if(ExternalVideoAcquisition)
    {
        return init_external_video(mbox);
    }
    if(!Has_VFW)
        return 0;

#if defined(_Windows) && defined(USE_VFW)
    VFWAquire *a = new VFWAquire;
    a->set_fail_reason("unknown");
    a->msg_box = mbox;
    if(a->init(TheVFWMgr,
               (MMChannel::get_mdi_client_window_callback)(0),
               RTUserDefaults.get_frame_interval(), VFWInvestigateData.get_automatic()) == 0)
    {
        fail_reason = a->get_fail_reason();
        delete a;
        return 0;
    }
    vspin();
    TheAq = a;
    vunlock();
    setupaq(VFWInvestigateData);
    // this is totally bogus. apparently with
    // miro's vfw drivers, if you don't start
    // acquisition the system crashes when you
    // try to stop things... this call
    // starts acquisition even though we don't
    // really need to.
    //
    a->need();

    return 1;
#else
    return 0;
#endif
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
#ifdef USE_VFW
        vspin();
#endif
        MMChannel::clean_video_sampler_refs(TheAq);
        // NOTE: NO CALLS TO VSPIN ALLOWED IN THE DTOR
        delete TheAq;
        TheAq = 0;
#ifdef USE_VFW
        vunlock();
#endif
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


void
setupaq(VFWInvestigateXfer& v)
{
#if defined(_Windows) && defined(USE_VFW)
    VFWAquire *va = TYPESAFE_DOWNCAST(TheAq, VFWAquire);
    if(!va)
        return;
    VFWAquire& a = *va;
    if(!v.get_automatic())
    {
        if(v.get_yuv9())
        {
            a.set_style(AQ_YUV9);
        }
        else if(v.get_yuv12())
        {
            a.set_style(AQ_YUV12);
        }
        else if(v.get_rgb16())
        {
            a.set_style(AQ_RGB555);
        }
        else if(v.get_rgb24())
        {
            a.set_style(AQ_RGB24);
        }
        else if(v.get_palette())
        {
            a.set_style(AQ_INDEX);
        }
        else if(v.get_uyvy())
        {
            a.set_style(AQ_UYVY);
        }
        else if(v.get_yuy2())
        {
            a.set_style(AQ_YUY2);
        }
    }
    if(!v.get_use_one_plane())
    {
        a.set_use_plane(AQ_ALL_PLANES);
    }
    else
    {
        a.set_use_plane(AQ_ONE_PLANE);
        if(v.get_red())
            a.set_which_plane(AQ_RED);
        else if(v.get_green())
            a.set_which_plane(AQ_GREEN);
        else if(v.get_blue())
            a.set_which_plane(AQ_BLUE);
    }
    a.set_upside_down(v.get_upside_down());
    unsigned int b_and, b_or, b_xor;
    int samp_off;
    v.get_masks(b_xor, b_and, b_or, samp_off);
    a.xor_mask = b_xor;
    a.and_mask = b_and;
    a.or_mask = b_or;
    a.samp_off = samp_off;
    a.autoconfig = v.get_automatic();
    a.swap_uv = v.get_swap_uv();
    if(v.get_enable_color())
        a.add_style(AQ_COLOR);
    else
        a.remove_style(AQ_COLOR);
#endif
}

