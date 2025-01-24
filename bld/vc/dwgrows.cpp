
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <memory.h>
#include "dwgrows.h"
#ifdef _MSC_VER
#include <string.h>
#endif

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/dwgrows.cpp 1.47 1997/11/06 21:37:56 dwight Stable $";

[[noreturn]] void oopanic(const char *);

DwGrowingString::DwGrowingString(int len)
    : str(len, !DWVEC_FIXED, DWVEC_AUTO_EXPAND, len)
{
	nmark = 0;
	curlen = 0;
}

long
DwGrowingString::length()
{
	return curlen;
}

// note: this call is a bit dicey, since the underlying vector
// is "not fixed and auto-resize" so, if the length is 0
// calling this will resize the vector to 1 and return
// a valid pointer (but to uninitialized stuff.) it is up to
// the caller to notice the length is 0 and not de-reference it.
// ie, don't be tempted to change this to "const", since that will
// change the behavior to "crash" instead of "return valid pointer to trash."
// this probably needs to be rethought a little bit...
const char *
DwGrowingString::ref_str()
{
	return &str[0];
}

long
DwGrowingString::copy_out(char *buf, long len)
{
    if(len > curlen)
        oopanic("copy_out len error");
	long movelen = len < curlen ? len : curlen;
	memmove(buf, &str[0], movelen);
	return movelen;
}

void
DwGrowingString::reset()
{
	curlen = 0;
	nmark = 0;
}

void
DwGrowingString::append(const char *s, long len)
{
	if(curlen + len > str.num_elems())
		str.set_size(curlen + len);
	char *a = &str[curlen];
	memmove(a, s, len);
	curlen += len;
}

void
DwGrowingString::consume_all_but(long len)
{
    if(len < 0)
        oopanic("bad len to consume");
    if(len >= curlen)
    {
        return;
    }
    memmove(&str[0], &str[curlen - len], len);
    curlen = len;
    nmark = 0;
}

void
DwGrowingString::mark()
{
	if(nmark >= NMARKS)
		oopanic("out of marks");
	mark_stack[nmark++] = curlen;
}

int
DwGrowingString::pop_to_mark(char *& buf_out, long& len_out)
{
	if(nmark <= 0)
		oopanic("mark underflow");
	long new_curlen = mark_stack[--nmark];
	buf_out = &str[new_curlen];
	len_out = curlen - new_curlen;
	curlen = new_curlen;
	return 1;
}

void
DwGrowingString::toss_mark()
{
	if(nmark <= 0)
		oopanic("mark  underflow");
	--nmark;
}

#ifdef TEST
#include <stdio.h>
void
oopanic(const char *)
{
::abort();
}
int
main(int, char **)
{

	DwGrowingString a(2);
	a.append("this is great", 5);
	
	printf("len %d\n", a.length());

	a.consume_all_but(1);
	printf("len %d\n", a.length());
	a.append("\0", 1);
	printf("str \"%s\"\n", a.ref_str());
}
#endif

