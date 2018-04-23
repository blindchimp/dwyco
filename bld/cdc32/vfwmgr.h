
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vfwmgr.h 1.5 1999/01/10 16:10:54 dwight Checkpoint $
 */
#ifndef VFWMGR_H
#define VFWMGR_H
#include <windows.h>
#include <vfw.h>

class VFWShit
{
public:
    VFWShit();
    virtual ~VFWShit();

    static int fire_up_vfw(int device);
    static int shutdown_vfw();

    virtual int create_capture_window(HWND h);
    virtual int preview_off();
    virtual int preview_on();
    virtual int start_preview();
    virtual int is_preview_on();
    virtual int change_driver(int idx);
    virtual int create_preview_child() = 0;
    virtual int put_vfw_preview_in_child() = 0;
    virtual int driver_connect(int idx);
    virtual int driver_disconnect();
    virtual int exit();
    virtual int change_format ();
    virtual int source_clicked ();
    virtual int make_capture_win_visible();
    virtual int make_capture_win_invisible();
    virtual int resize_preview_window() = 0;
    virtual HWND get_main_window() = 0;
    virtual HWND get_client_window() = 0;
    virtual void set_cursor(int) = 0;
    virtual int is_hardware_capturing();


    HWND capwnd;
    CAPDRIVERCAPS caps;
    int preview_id;
    int cur_idx;
};

extern VFWShit *TheVFWMgr;
#endif
