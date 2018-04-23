
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "pfx.h"


DwOString User_pfx;
DwOString Sys_pfx;
DwOString Tmp_pfx;

DwOString
add_pfx(const DwOString& pfx, const DwOString& fn)
{
    DwOString ret = pfx;
    ret += "/";
    ret += fn;
    return ret;
}
