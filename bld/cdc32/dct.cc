
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dct.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "matcom.h"

void prepare_range_limit_table();
void
init_dct()
{
    prepare_range_limit_table();
}
