
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef DWYCO_NO_VCREGEX
#include "vcdeflt.h"
#include "vcregex.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcregex.cpp 1.45 1996/11/17 05:59:03 dwight Stable $";

int
vc_regex::re_match(const vc& v) const
{
	const char *s = v.peek_str();

	return re.match(s, strlen(s));

}
int
vc_regex::re_match(const vc& v, int len, int pos) const
{
	const char *s = v.peek_str();

	return re.match(s, len, pos);

}

int
vc_regex::re_search(const vc& v, int len, int& matchlen, int startpos) const
{
	const char *s = v.peek_str();

    return re.search(s, len, matchlen, startpos);
}


int
vc_regex::re_match_info(int& start, int& length, int nth) const
{
	return re.match_info(start, length, nth);
}


void
vc_regex::stringrep(VcIO os) const
{
	os << "regex(";
	if(re.error()) 
		os << "INVALID ";
	vc_string::stringrep(os);
	os << ")";
}
#endif
