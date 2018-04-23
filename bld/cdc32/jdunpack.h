
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jdunpack.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef JDUNPACK_H
#define JDUNPACK_H

#include "packbits.h"
#include "jdhuff.h"

class JDUnpackbits : public Unpackbits
{
public:
    int huff_decode(d_derived_tbl *htbl, BITBUFT*& inbuf);
    int slow_decode(d_derived_tbl *htbl, int minbits, BITBUFT*& inbuf);
private:
};

#endif
