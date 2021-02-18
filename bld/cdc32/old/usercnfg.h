
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/usercnfg.h 1.16 1999/01/10 16:10:23 dwight Checkpoint $
 */
#if !defined(__usercnfg_h)              // Sentry, use file only if it's not already included.
#define __usercnfg_h

#include "uicfg.h"

struct UserConfigXfer {
public:
    UserConfigXfer();
    void save();
    void load();
    int is_saved();

    DWUIDECL_BEGIN
    DWUIDECLVAL(const char *, description)
    DWUIDECLVAL(const char *, username)
    DWUIDECLVAL(const char *, email)
    DWUIDECLVAL(const char *, location)
    DWUIDECL_END

};

extern UserConfigXfer UserConfigData;

#endif                                      // __usercnfg_h sentry.

