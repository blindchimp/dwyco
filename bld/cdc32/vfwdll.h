
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
#ifndef VFWDLL_H
#define VFWDLL_H
#include <windows.h>
#include <vfw.h>
#include "vfwmgr.h"

class VFWShitDLL : public VFWShit
{
public:
    VFWShitDLL();

    static int fire_up_vfw(int device, HWND main_hwnd, HWND client_hwnd);

    int create_preview_child();
    int put_vfw_preview_in_child();
    int resize_preview_window();
    void set_cursor(int);
    HWND get_main_window();
    HWND get_client_window();

    HWND ext_main_hwnd;
    HWND ext_client_hwnd;
    HWND ext_child_hwnd;
};

#endif
