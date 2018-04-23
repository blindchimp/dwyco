
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vidinput.h 1.19 1999/01/10 16:10:24 dwight Checkpoint $
 */
#if !defined(__vidinput_h)              // Sentry, use file only if it's not already included.
#define __vidinput_h


#include "uicfg.h"

struct VidInputXfer {
public:
    VidInputXfer();

    int get_device();


    DWUIDECL_BEGIN
    DWUIDECLVAL(const char *, device_name)
    DWUIDECLVAL(bool, coded)
    DWUIDECLVAL(bool, raw)
    DWUIDECLVAL(bool, vfw)
    DWUIDECLVAL(bool, no_video)
    DWUIDECLVAL(int, device_index)
    DWUIDECL_END


    void load();
    void save();
private:
    void setup_radio(int);
};

extern VidInputXfer VidInputData;
#endif                                      // __vidinput_h sentry.

