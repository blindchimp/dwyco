
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCSC_H
#define VCSC_H
// $Header: g:/dwight/repo/vc/rcs/vcsc.h 1.45 1996/11/17 05:59:07 dwight Stable $

class VcSrcCoord
{
public:
};

class VcSrcCoordString : public VcSrcCoord
{
private:

public:
	const char *start;
	const char *end;
	long idx_start;
	long idx_end;
};




#endif
