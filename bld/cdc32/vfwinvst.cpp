
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vfwinvst.cpp 1.8 1997/11/25 20:41:03 dwight Stable095 $
 */

#include "vfwinvst.h"

VFWInvestigateXfer VFWInvestigateData;
VFWInvestigateXfer VIDefaults;

#define MYSTERY_VFW_SECTION "mystery vfw"

#define DEFAULT_BLUE 0
#define DEFAULT_GREEN 0
#define DEFAULT_RED 1
#define DEFAULT_RGB16 0
#define DEFAULT_RGB24 0
#define DEFAULT_YUV9 0
#define DEFAULT_YUV12 0
#define DEFAULT_YUY2 0
#define DEFAULT_UYVY 0
#define DEFAULT_AUTOMATIC 1
#define DEFAULT_PALETTE 0
#define DEFAULT_UPSIDE_DOWN 0
#define DEFAULT_USE_ONE_PLANE 0
#define DEFAULT_DEVICE "0"
#define DEFAULT_AND "0xff"
#define DEFAULT_OR "0"
#define DEFAULT_XOR "0"
#define DEFAULT_OFFSET "0"
#define DEFAULT_ENABLE_COLOR BF_CHECKED
#define DEFAULT_SWAP_UV 0
#define DEFAULT_SWAP_RB 0

#define BLUE "blue"
#define RED "red"
#define GREEN "green"
#define RGB16 "rgb16"
#define RGB24 "rgb24"
#define YUV9 "yuv9"
#define YUV12 "yuv12"
#define YUY2 "yuy2"
#define UYVY "uyvy"
#define SWAP_UV "swap_uv"
#define AUTOMATIC "automatic"
#define UPSIDE_DOWN "upsidedown"
#define USE_ONE_PLANE "useoneplane"
#define DEVICE "device"
#define VFW_AND "b_and"
#define VFW_OR "b_or"
#define VFW_XOR "b_xor"
#define VFW_OFFSET "offset"
#define PALETTE "palette"
#define ENABLE_COLOR "color"
#define SWAP_RB "swap_rb"

VFWInvestigateXfer::VFWInvestigateXfer()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(blue),
DWUIINIT_CTOR_VAL(green),
DWUIINIT_CTOR_VAL(red),
DWUIINIT_CTOR_VAL(rgb16),
DWUIINIT_CTOR_VAL(rgb24),
DWUIINIT_CTOR_VAL(yuv9),
DWUIINIT_CTOR_VAL(yuv12),
DWUIINIT_CTOR_VAL(swap_uv),
DWUIINIT_CTOR_VAL(automatic),
DWUIINIT_CTOR_VAL(palette),
DWUIINIT_CTOR_VAL(upside_down),
DWUIINIT_CTOR_VAL(use_one_plane),
DWUIINIT_CTOR_VAL(enable_color),
DWUIINIT_CTOR_VAL(device),
DWUIINIT_CTOR_VAL(b_and),
DWUIINIT_CTOR_VAL(b_or),
DWUIINIT_CTOR_VAL(b_xor),
DWUIINIT_CTOR_VAL(yuy2),
DWUIINIT_CTOR_VAL(uyvy),
DWUIINIT_CTOR_VAL(offset),
DWUIINIT_CTOR_VAL(swap_rb)
DWUIINIT_CTOR_END
{
    set_blue(DEFAULT_BLUE);
    set_green(DEFAULT_GREEN);
    set_red(DEFAULT_RED);
    set_rgb16(DEFAULT_RGB16);
    set_rgb24(DEFAULT_RGB24);
    set_yuv9(DEFAULT_YUV9);
    set_yuv12(DEFAULT_YUV12);
    set_yuy2(DEFAULT_YUY2);
    set_uyvy(DEFAULT_UYVY);
    set_swap_uv(DEFAULT_SWAP_UV);
    set_automatic(DEFAULT_AUTOMATIC);
    set_palette(DEFAULT_PALETTE);
    set_upside_down(DEFAULT_UPSIDE_DOWN);
    set_use_one_plane(DEFAULT_USE_ONE_PLANE);
    set_enable_color(DEFAULT_ENABLE_COLOR);
    set_device(DEFAULT_DEVICE);
    set_b_and(DEFAULT_AND);
    set_b_or(DEFAULT_OR);
    set_b_xor(DEFAULT_XOR);
    set_offset(DEFAULT_OFFSET);
    set_swap_rb(DEFAULT_SWAP_RB);
}
#ifdef LEAK_CLEANUP_NOPERS
VFWInvestigateXfer::~VFWInvestigateXfer()
{
    delete syncmap;
    syncmap = 0;
}
#endif

const VFWInvestigateXfer&
VFWInvestigateXfer::operator=(VFWInvestigateXfer& v)
{
    if(this == &v)
        return *this;
    set_blue(v.get_blue());
    set_green(v.get_green());
    set_red(v.get_red());
    set_rgb16(v.get_rgb16());
    set_rgb24(v.get_rgb24());
    set_yuv9(v.get_yuv9());
    set_yuv12(v.get_yuv12());
    set_yuy2(v.get_yuy2());
    set_uyvy(v.get_uyvy());
    set_swap_uv(v.get_swap_uv());
    set_automatic(v.get_automatic());
    set_upside_down(v.get_upside_down());
    set_use_one_plane(v.get_use_one_plane());
    set_palette(v.get_palette());
    set_enable_color(v.get_enable_color());
    set_device(v.get_device());
    set_b_or(v.get_b_or());
    set_b_and(v.get_b_and());
    set_b_xor(v.get_b_xor());
    set_offset(v.get_offset());
    set_swap_rb(v.get_swap_rb());
    return *this;
}


void
VFWInvestigateXfer::get_masks(unsigned int& xor_out, unsigned int& and_out,
                              unsigned int& or_out, int& off_out)
{
    xor_out = strtol(get_b_xor(), 0, 0);
    and_out = strtol(get_b_and(), 0, 0);
    or_out = strtol(get_b_or(), 0, 0);
    off_out = strtol(get_offset(), 0, 0);
}

void
VFWInvestigateXfer::save()
{
    save_syncmap(syncmap, MYSTERY_VFW_SECTION ".dif");
}

void
VFWInvestigateXfer::load()
{
    load_syncmap(syncmap, MYSTERY_VFW_SECTION ".dif");

}


