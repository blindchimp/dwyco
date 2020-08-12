
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vcdbg.h"

#ifdef VCDBG
#include "dwdate.h"
#include "vcmap.h"

// note: there used to be a bunch of stuff in here for dropping to
// an interactive debugging shell. it sorta worked, but it was never
// terribly useful. i'm leaving the backtrace stuff in here mainly because
// it can be useful to have a snapshot of the state periodically.
//
// i think a much more useful debugging facility might be streaming
// state information to an external source, since a lot of what LH
// ended up being used for was small distributed services. it might also
// be possible to provide some kind of "go backwards" or reset to previous state
// functionality. interesting ideas, but probably never going to get done.
//

int
drop_to_dbg(const char *, const char *)
{
oopanic("no debugger");
}

int Eval_break;
VcDebugInfo VcDbgInfo;

void
VcDebugInfo::backtrace(VcIO os)
{
	os << "---Execution Backtrace---\n";
	long i = callstack.num_elems() - 1;
	for(; i >= 0; --i)
	{
		callstack[i]->printOn(os);
	}
	os << "---End Backtrace---\n";
}

void
VcDebugInfo::backtrace_brief(VcIO os, int start, int num) 
{
	long i = callstack.num_elems() - 1;
	int frm_cnt = 0;
	int cnt = 0;
	for(; i >= 0; --i)
	{
		if(callstack[i]->has_brief())
		{
			if(num != -1 && cnt++ >= num)
				break;
			++frm_cnt;
			if(frm_cnt >= start)
			{
				os << "#" << frm_cnt << " ";
				callstack[i]->printOnBrief(os);
			}
		}
	}
}

VcDebugNode *
VcDebugInfo::get(int n)
{
	long i = callstack.num_elems();
	if(i == 0)
		return 0;
	if(n == -1)
	{
		return callstack[i - 1];
	}
	if(n >= i)
		return 0;
	return callstack[n];
}

	
VcDebugNode::VcDebugNode()
{
	VcDbgInfo.callstack.append(this);
	filename = "<<unknown>>";
	linenum = 0;
}

VcDebugNode::~VcDebugNode()
{
	long i = VcDbgInfo.callstack.index(this);
	if(i == -1)
		oopanic("can't find debug node?");
	VcDbgInfo.callstack.set_size(i);
}

void
VcDebugNode::printOn(VcIO o)
{
	DwDate d;
	DwTime t;
	o << d.Month() << "/" << d.DayOfMonth() << "/" << d.Year() << " " <<
		t.AsString().c_str() << " ";
}

void
VcDebugNode::printOnBrief(VcIO o)
{
	VcDebugNode::printOn(o);
}

//
// LH debugging hooks
//

vc
vclh_break_on_call(vc fun)
{
	switch(fun.type())
	{
	case VC_FUNC:
		fun.set_break_on(BREAK_CALL);
		break;
	case VC_MEMFUN:
		fun.set_break_on(BREAK_MEMCALL);
		break;
	default:
		USER_BOMB("can't set call break on non-function", vcnil);
	}

	return vcnil;
}

vc
vclh_break_on_access(vc thing)
{
	thing.set_break_on(BREAK_ACCESS);
	return vcnil;
}


vc
vclh_eval_break_on(vc what)
{
  vc::set_global_break_on(1);
  return vcnil;
}

vc
vclh_eval_break_off(vc what)
{
  vc::clear_global_break_on(1);
  return vcnil;
}

#endif
