
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vidinput.cpp 1.23 1999/01/10 16:10:04 dwight Checkpoint $
 */

#include <string.h>

#include "vidinput.h"

VidInputXfer VidInputData;
#define VIDINPUT_SECTION "video input"
#define VIDINPUT_DEFAULT VIDINPUT_NO_VIDEO
#define VIDINPUT_DEVICE_DEFAULT ""

#define VIDINPUT "input from"
#define VIDINPUT_DEVICE "device_index"
#define VIDINPUT_RAW 0
#define VIDINPUT_CODED 1
#define VIDINPUT_VFW 2
#define VIDINPUT_NO_VIDEO 3

VidInputXfer::VidInputXfer()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(coded),
DWUIINIT_CTOR_VAL(raw),
DWUIINIT_CTOR_VAL(vfw),
DWUIINIT_CTOR_VAL(no_video),
DWUIINIT_CTOR_VAL(device_name),
DWUIINIT_CTOR_VAL(device_index)
DWUIINIT_CTOR_END
{
    setup_radio(VIDINPUT_DEFAULT);
}

void
VidInputXfer::setup_radio(int a)
{
    set_coded(0);
    set_raw(0);
    set_vfw(0);
    set_no_video(0);
    switch(a)
    {
    case VIDINPUT_RAW:
        set_raw(1);
        break;
    case VIDINPUT_CODED:
        set_coded(1);
        break;
    case VIDINPUT_VFW:
        set_vfw(1);
        break;
    case VIDINPUT_NO_VIDEO:
        set_no_video(1);
        break;
    }
}

void
VidInputXfer::load()
{
    load_syncmap(syncmap, VIDINPUT_SECTION ".dif");
}

void
VidInputXfer::save()
{
    save_syncmap(syncmap, VIDINPUT_SECTION ".dif");
}

int
VidInputXfer::get_device()
{
    return get_device_index();
}

