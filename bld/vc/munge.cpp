
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/vc/rcs/munge.cpp 1.3 1997/10/05 17:27:06 dwight Stable $
 */
#include <stdio.h>
#ifndef __GNUG__
#include <fcntl.h>
#include <io.h>
#endif
#include "enc.h"

main()
{
	int c;
	Enc e;
#ifndef __GNUG__
	setmode(0, O_BINARY);
	setmode(1, O_BINARY);
#endif
	while((c = getchar()) != EOF)
	{
		char b = c;
		e.munge(&b, 1);
		putchar(b);
	}
}
