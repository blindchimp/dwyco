
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcfile.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfile.cpp 1.14 1997/11/27 03:06:05 dwight Stable $";

int
vc_file::do_error_processing(vc exc, vc a1, vc a2)
{
	errvc = exc;
	errvc1 = a1;
	errvc2 = a2;
	if(err_callback != 0)
		return (*err_callback)(this);
	else
		return VC_FILE_RESUME;
}

long
vc_file::overflow(vcxstream&, char *buf, long len)
{
	if(!write(buf, len))
		return -1;
	return len;
}

long
vc_file::underflow(vcxstream&, char *buf, long min, long max)
{
	long got = min;
	if(!read(buf, got))
		return -1;
	if(got < min)
		return -1;
    return got;
}
