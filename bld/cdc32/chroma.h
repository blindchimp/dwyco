
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/chroma.h 1.6 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef CHROMA_H
#define CHROMA_H

#include "jtcode.h"

class JPEGTCoderChroma : public JPEGTCoder
{
public:
    JPEGTCoderChroma(JPEGTCoder *l = 0);
    virtual ~JPEGTCoderChroma();

    void code_frame(gray **frm, int cols, int rows, int is_ref, const char *polname);

private:
    JPEGTCoder *luma;
    int fsampling;
};

#define CHROMA_HEADER_SZ sizeof(unsigned short)

#endif
