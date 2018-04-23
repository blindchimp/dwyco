
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCPROF_H
#define VCPROF_H
#include "vc.h"
#ifdef LHPROF
// $Header: g:/dwight/repo/vc/rcs/vcdbg.h 1.47 1997/11/27 03:06:00 dwight Stable $

#include "dwvecp.h"

// class used to manage profiling info in an
// LH program. 

class VcProfileNode;

typedef DwVecP<VcProfileNode> VcProfNodeVec;
typedef DwVec<vc> VcVec;

class VcProfileInfo
{
friend class VcProfileNode;

private:
	VcProfNodeVec callstack;

public:
	virtual ~VcProfileInfo() {}
	virtual VcProfileNode *get(int n = -1);
	virtual int num_elems() {return callstack.num_elems();}

};

struct VcProfileInfo
{
	VcProfileInfo() {
		call_count = 0;
		total_time = 0;
		total_time2 = 0;
		total_sys_time = 0;
		total_sys_time2 = 0;
		total_real_time = 0;
		total_real_time2 = 0;
	}

	double call_count;
	double total_time;
	double total_time2;
	double total_sys_time;
	double total_sys_time2;
	double total_real_time;
	double total_real_time2;
};

class VcProfileNode
{
public:
	DwAMap<VcProfileInfo, vc> callees;

	VcProfileNode();
	virtual ~VcProfileNode();
};

extern VcProfileInfo VcProfInfo;


#endif
#endif
