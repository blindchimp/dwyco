
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vfwmgr.cc 1.10 1999/01/10 16:09:41 dwight Checkpoint $
 */
#include "vfwmgr.h"
#include "gvchild.h"
#include "aq.h"
#include "aqvfw.h"
#include "sleep.h"
#include "vfwinvst.h"

// note: potty mouth in this class (and subclasses) is due to years of
// frustration with VFW (and crummy drivers). i'm glad VFW is mostly
// gone from the planet...

extern int Sleeping;
VFWShit *TheVFWMgr;

VFWShit::VFWShit()
{
    capwnd = 0;
    preview_id = 0;
}

VFWShit::~VFWShit()
{

    if(TheAq)
    {
        VFWAquire *va = TYPESAFE_DOWNCAST(TheAq, VFWAquire);
        if(va)
        {
            va->stop();
            va->disconnect();
            va->vfwmgr = 0; // we know we are the only associated vfwmgr
            dwsleep(1);
        }
    }
    if(capwnd)
    {
        capPreview(capwnd, 0);
        capOverlay(capwnd, 0);
        driver_disconnect();
        DestroyWindow(capwnd);
        capwnd = 0;
    }
}

static LRESULT CALLBACK
error(HWND hwnd, int id, LPCSTR txt)
{
    if(id == 0)
        return 0;
    MessageBox(hwnd, txt, "vidinp: video for windows error", MB_OK|MB_ICONSTOP);
    return 1;
}

int
VFWShit::create_capture_window(HWND h)
{
    capwnd = capCreateCaptureWindow("Dwyco Video", WS_CHILD,
                                    0, 0, 160, 120, get_main_window(), 0);
    capSetCallbackOnError(capwnd, ::error);
    return 1;
}

int
VFWShit::make_capture_win_visible()
{
    if(!capwnd)
        return 0;
    ::SetWindowLong(capwnd, GWL_STYLE,
                    ::GetWindowLong(capwnd, GWL_STYLE) | WS_VISIBLE);
    return 1;
}

int
VFWShit::make_capture_win_invisible()
{
    if(!capwnd)
        return 1;
    ::SetWindowLong(capwnd, GWL_STYLE,
                    ::GetWindowLong(capwnd, GWL_STYLE) & ~WS_VISIBLE);
    return 1;
}

int
VFWShit::is_preview_on()
{
    return preview_id != 0;
}

int
VFWShit::preview_off()
{

    if(capwnd)
    {
        capPreview(capwnd, 0);
        capOverlay(capwnd, 0);
        // keep the capture window from being destroyed
        make_capture_win_invisible();
        ::SetParent(capwnd, get_main_window());
        preview_id = 0;
    }
    return 1;
}

int
VFWShit::start_preview()
{
    create_preview_child();
    put_vfw_preview_in_child();
    make_capture_win_visible();
    preview_on();
    return 1;
}


int
VFWShit::change_driver(int idx)
{
    if(cur_idx == idx)
        return 1;
    int old_idx = driver_disconnect();
    if(!driver_connect(idx))
    {
        driver_connect(old_idx);
        return 0;
    }
    return 1;
}



#define FAIL {set_cursor(1); return 0;}
int
VFWShit::driver_connect(int idx)
{
    set_cursor(0);
    if(capwnd == NULL)
    {
        error(get_client_window(), 1, "can't create capture window");
        FAIL
    }
    if(!capDriverConnect(capwnd, idx))
    {
        FAIL
    }
    if(!capDriverGetCaps(capwnd, &caps, sizeof(caps)))
    {
        FAIL
    }
    capOverlay(capwnd, 0);
    if(!capPreviewRate(capwnd, 99))
    {
        error(get_client_window(), 1, "warning: can't set preview rate");
    }
    set_cursor(1);
    cur_idx = idx;
    return 1;
}
#undef FAIL

int
VFWShit::preview_on()
{
    if(!capwnd)
        return 0;
#ifdef USE_VFW_OVERLAY
    if(!capOverlay(capwnd, 1))
#else
    if(!capPreview(capwnd, 1))
#endif
    {
        error(get_client_window(), 1, "can't enable preview");
        return 0;
    }
    return 1;
}

int
VFWShit::driver_disconnect()
{
    capSetCallbackOnError(capwnd, NULL);
    capDriverDisconnect(capwnd);
    int tmp = cur_idx;
    cur_idx = -1;
    return tmp;
}

int
VFWShit::exit()
{
    if(!capwnd)
        return 1;
    driver_disconnect();
    DestroyWindow(capwnd);
    capwnd = 0;
    return 1;
}

int
VFWShit::is_hardware_capturing()
{
    if(capwnd)
    {
        CAPSTATUS c;
        capGetStatus(capwnd, &c, sizeof(c));
        return c.fCapturingNow;
    }
    return 0;
}

int
VFWShit::change_format ()
{
    // INSERT>> Your code here.
    if(Sleeping)
        return 0;
    if(!capwnd)
        return 0;
    VFWAquire *va = 0;
    if(TheAq)
        va = TYPESAFE_DOWNCAST(TheAq, VFWAquire);
    int restart = 0;
    if(va && va->streaming)
    {
#if 0
        MessageBox("You must disconnect all sessions before changing video format");
        return;
#endif
        va->stop();
#if 0
        restart = 1;
        for(int i = 0; i < 5; ++i)
        {
            CAPSTATUS c;
            capGetStatus(capwnd, &c, sizeof(c));
            if(c.fCapturingNow)
            {
                dwsleep(1);
            }
            else
                break;
        }
#endif
    }


    int do_start_preview = 0;
    if(preview_id != 0)
    {
        preview_off();
        do_start_preview = 1;
    }

    if(!caps.fHasDlgVideoFormat)   // shouldn't be necessary, but...
        error(get_client_window(), 1, "driver cannot change format");
    else if(!capDlgVideoFormat(capwnd))
        error(get_client_window(), 1, "can't get format");


    if(do_start_preview)
    {
        start_preview();
        resize_preview_window();
    }

    if(va)
    {
        va->new_format();
        setupaq(VFWInvestigateData);
        if(restart)
            va->need();
    }
    return 1;

}


int VFWShit::source_clicked ()
{
    // INSERT>> Your code here.
    if(Sleeping)
        return 0;
    if(!capwnd)
        return 0;
    VFWAquire *va = 0;
    if(TheAq)
        va = TYPESAFE_DOWNCAST(TheAq, VFWAquire);
    int restart = 0;
    int got_setup = 0;
    CAPTUREPARMS capsetup;
    if(va && va->streaming)
    {
        va->stop();
        restart = 1;
#if 0
        while(1)
        {
            CAPSTATUS c;
            capGetStatus(capwnd, &c, sizeof(c));
            if(c.fCapturingNow)
            {
                dwsleep(1);
            }
            else
                break;
        }
        //got_setup = capCaptureGetSetup(capwnd, &capsetup, sizeof(capsetup));
#endif
    }
    int stop_preview = 0;
    if(preview_id == 0)
    {
        start_preview();
        stop_preview = 1;
    }
    // popup preview window
    if(!caps.fHasDlgVideoSource)   // shouldn't be necessary, but...
        error(get_client_window(), 1, "driver cannot change video source");
    else if(!capDlgVideoSource(capwnd))
        error(get_client_window(), 1, "can't get source");
    if(va)
    {
        va->new_format();
        setupaq(VFWInvestigateData);
        if(restart)
        {
#if 0
            if(got_setup)
                capCaptureSetSetup(capwnd, &capsetup, sizeof(capsetup));
#endif
            va->need();
        }
    }
    if(stop_preview)
        preview_off();
    return 1;
}


int
VFWShit::fire_up_vfw(int device)
{
    return 1;
}

int
VFWShit::shutdown_vfw()
{
    delete TheVFWMgr;
    TheVFWMgr = 0;
    return 1;
}
