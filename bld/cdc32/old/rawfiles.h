
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/rawfiles.h 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */
#if !defined(__rawfiles_h)              // Sentry, use file only if it's not already included.
#define __rawfiles_h
#include "uicfg.h"


struct RawFilesXfer {
public:
    RawFilesXfer();
    void load();
    void save();

    DWUIDECL_BEGIN
    DWUIDECLVAL(const char *, raw_files_list)
    DWUIDECLVAL(const char *, raw_files_pattern)
    DWUIDECLVAL(bool, use_list_of_files)
    DWUIDECLVAL(bool, use_pattern)
    DWUIDECLVAL(bool, preload)
    DWUIDECL_END
};

extern RawFilesXfer RawFilesData;
#endif                                      // __rawfiles_h sentry.

