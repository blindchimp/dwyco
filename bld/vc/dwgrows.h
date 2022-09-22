
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWGROWS_H
#define DWGROWS_H
// $Header: g:/dwight/repo/vc/rcs/dwgrows.h 1.47 1997/11/06 21:37:56 dwight Stable $

#include "dwvec.h"
#define NMARKS 5

class DwGrowingString
{
public:
        DwGrowingString(int init_len = 128);
    // warning: this invalidates any pointers gotten by
    // ref_str
	void append(const char *, long len);
	void mark();
	int pop_to_mark(char *& buf_out, long& len_out);
	void toss_mark();
	long length();
	const char *ref_str();
	long copy_out(char *, long len);
	void reset();
    // without changing the size of the internal buffer,
    // move the last chunk of data to the
    // beginning of the string, and reset the size
    // to len.
    void consume_all_but(long len);

private:

	int nmark;
	long curlen;
	long mark_stack[NMARKS];
	DwVec<char> str;
};

#endif
