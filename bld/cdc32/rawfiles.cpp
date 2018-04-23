
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/rawfiles.cpp 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */

#include "rawfiles.h"
#include "doinit.h"

#define RAWFILES_SECTION "raw files"
#define USE_LIST_OF_FILES_DEFAULT 1
#define USE_PATTERN_DEFAULT 0
#define PRELOAD_DEFAULT 0
#define RAW_FILES_LIST_DEFAULT ""
#define RAW_FILES_PATTERN_DEFAULT ""

#define USE_LIST_OF_FILES "use list of files"
#define USE_PATTERN "use pattern"
#define PRELOAD "preload"
#define RAW_FILES_LIST "list file"
#define RAW_FILES_PATTERN "pattern"

RawFilesXfer::RawFilesXfer()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(use_list_of_files),
DWUIINIT_CTOR_VAL(use_pattern),
DWUIINIT_CTOR_VAL(preload),
DWUIINIT_CTOR_VAL(raw_files_list),
DWUIINIT_CTOR_VAL(raw_files_pattern)
DWUIINIT_CTOR_END
{
    set_use_list_of_files(1);
    set_use_pattern(0);
    set_preload(0);
    set_raw_files_list("");
    set_raw_files_pattern("");
}

void
RawFilesXfer::save()
{
    save_syncmap(syncmap, RAWFILES_SECTION ".dif");
}

void
RawFilesXfer::load()
{
    load_syncmap(syncmap, RAWFILES_SECTION ".dif");

}
RawFilesXfer RawFilesData;

