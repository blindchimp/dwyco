
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vfwinvst.h 1.8 1997/11/25 20:41:03 dwight Stable095 $
 */
#if !defined(__vfwinvst_h)              // Sentry, use file only if it's not already included.
#define __vfwinvst_h


#include "uicfg.h"

//{{TDialog = VFWInvestigate}}
struct VFWInvestigateXfer {
public:
//{{VFWInvestigateXFER_DATA_END}}
    VFWInvestigateXfer();
#ifdef LEAK_CLEANUP_NOPERS
    ~VFWInvestigateXfer();
#endif
    const VFWInvestigateXfer& operator=(VFWInvestigateXfer&);
    void get_masks(unsigned int& b_xor, unsigned int& b_and, unsigned int& b_or, int& samp_off);
    void save();
    void load();

    DWUIDECL_BEGIN
    DWUIDECLVAL(const char *, device)
    DWUIDECLVAL(const char *, b_and)
    DWUIDECLVAL(const char *, b_or)
    DWUIDECLVAL(const char *, b_xor)
    DWUIDECLVAL(const char *, offset)
    DWUIDECLVAL(bool, blue)
    DWUIDECLVAL(bool, green)
    DWUIDECLVAL(bool, red)
    DWUIDECLVAL(bool, rgb16)
    DWUIDECLVAL(bool, rgb24)
    DWUIDECLVAL(bool, use_one_plane)
    DWUIDECLVAL(bool, yuv9)
    DWUIDECLVAL(bool, upside_down)
    DWUIDECLVAL(bool, palette)
    DWUIDECLVAL(bool, automatic)
    DWUIDECLVAL(bool, enable_color)
    DWUIDECLVAL(bool, yuv12)
    DWUIDECLVAL(bool, swap_uv)
    DWUIDECLVAL(bool, uyvy)
    DWUIDECLVAL(bool, yuy2)
    DWUIDECLVAL(bool, swap_rb)
    DWUIDECL_END

private:
    VFWInvestigateXfer(const VFWInvestigateXfer&);
};

extern VFWInvestigateXfer VFWInvestigateData;
extern VFWInvestigateXfer VIDefaults;

#endif                                      // __vfwinvst_h sentry.

