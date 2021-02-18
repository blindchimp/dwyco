
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
#include "aq.h"
#include "aqvfw.h"
#include "sleep.h"
#include "vfwinvst.h"
#include "vfwdll.h"

extern int Sleeping;

VFWShitDLL::VFWShitDLL()
{
    capwnd = 0;
    preview_id = 0;
    //preview_win = 0;
    ext_client_hwnd = 0;
    ext_main_hwnd = 0;
    ext_child_hwnd = 0;
}

#if 0
VFWShitDLL::~VFWShitDLL()
{

    if(TheAq)
    {
        VFWAquire *va = TYPESAFE_DOWNCAST(TheAq, VFWAquire);
        if(va)
        {
            va->stop();
            va->disconnect();
            dwsleep(1);
        }
    }
    if(capwnd)
    {
        driver_disconnect();
        DestroyWindow(capwnd);
        capwnd = 0;
    }
}

#endif

static LRESULT CALLBACK
error(HWND hwnd, int id, LPCSTR txt)
{
    if(id == 0)
        return 0;
    MessageBox(hwnd, txt, "video for windows error", MB_OK|MB_ICONSTOP);
    return 1;
}



int
VFWShitDLL::create_preview_child()
{
    // reattach a preview window to existing
    // capwindow
    if(preview_id)
        return 1;
    if(!ext_child_hwnd)
        return 0;
    preview_id = 1;
    ::SetWindowLong(ext_child_hwnd, GWL_STYLE, (::GetWindowLong(ext_child_hwnd, GWL_STYLE) & ~WS_SYSMENU)|WS_DISABLED);
    return 1;
}

int
VFWShitDLL::resize_preview_window()
{
    CAPSTATUS c;
    if(!capwnd)
        return 0;
    capGetStatus(capwnd, &c, sizeof(c));
    if(!ext_child_hwnd)
        return 0;
    //::MoveWindow(ext_child_hwnd, 0, 0, c.uiImageWidth, c.uiImageHeight, 1);
    ::MoveWindow(capwnd, 0, 0, c.uiImageWidth, c.uiImageHeight, 1);
    return 1;
}

int
VFWShitDLL::put_vfw_preview_in_child()
{
    resize_preview_window();
    if(!ext_child_hwnd)
        return 0;
    ::SetParent(capwnd, ext_child_hwnd);
    ::SetWindowLong(capwnd, GWL_STYLE,
                    ::GetWindowLong(capwnd, GWL_STYLE)|WS_VISIBLE);
#ifdef USE_VFW_OVERLAY
    capOverlay(capwnd, 1);
#else
    capPreview(capwnd, 1);
#endif
    return 1;
}

void
VFWShitDLL::set_cursor(int val)
{
}

HWND
VFWShitDLL::get_main_window()
{
    return ext_main_hwnd;
}

HWND
VFWShitDLL::get_client_window()
{
    return ext_client_hwnd;
}

int
VFWShitDLL::fire_up_vfw(int device, HWND main_hwnd, HWND client_hwnd)
{
    if(TheVFWMgr)
        shutdown_vfw();
    VFWShitDLL *d;
    TheVFWMgr = d = new VFWShitDLL;
    d->ext_main_hwnd = main_hwnd;
    d->ext_client_hwnd = client_hwnd;
    TheVFWMgr->create_capture_window(0);
    if(!TheVFWMgr->driver_connect(device))
    {
        ::MessageBox(TheVFWMgr->get_client_window(), "can't connect to video device, try selecting"
                     " a different device from the Setup|Video input dialog...", "video for windows error",
                     MB_OK|MB_ICONEXCLAMATION);

        shutdown_vfw();
        return 0;
    }
    //TheVFWMgr->start_preview();
    return 1;
}

