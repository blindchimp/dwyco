#ifdef DWYCO_DCT_CODER
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/packbits.cc 1.2 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "packbits.h"

#ifdef RECORD
#include "dwlista.h"
DwListA<int> Symlist;
Packbits *CTracking;
#endif
Packbits::Packbits()
{
    bits_left = BITBUFSZ;
}
#endif
