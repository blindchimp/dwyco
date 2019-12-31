
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

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcdbg.cpp 1.49 1998/06/11 06:53:35 dwight Exp $";


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
// drop into a debug shell
//
// "why" is a string indicating the conditions that
// brought us here. should be one of
// "bomb" (user error)
// "breakpoint" (for debug breaks)
// "toplev" (for initial debugger invocation)
//
int
drop_to_dbg(const char *msg, const char *why)
{
	vc la;
	vc fa;
	vc w(why);
	vc m(msg);
	VcDebugNode *dbgn = VcDbgInfo.get();
	if(dbgn == 0)
	{
		la = vczero;
		fa = vcnil;
	}
	else
	{
		la = dbgn->linenum;
        fa = dbgn->filename.c_str();
	}
#ifdef VCDBG_INTERACTIVE
	vc fun = Vcmap->get("__lh_debug");
	if(fun.type() != VC_FUNC)
	{
		fprintf(stderr, "debugger function failure");
		fflush(stderr);
		exit(1);
	}
	VCArglist a;
	a[0] = m;
	a[1] = w;
	a[2] = fa;
	a[3] = la;
    Eval_break = 0; // don't break in debugger
	vc v = fun(&a);
	if(v != vc("cont") && !Vcmap->backout_in_progress())
#else
	VcError << msg << "\n";
#endif
		Vcmap->set_dbg_backout();
	CHECK_ANY_BO(1);
    vc v2 = Vcmap->get("__lh_single_step");
    if(v2.is_nil())
      Eval_break = 0;
    else
      Eval_break = 1;
	return 0;
}

#if 0
int
drop_to_dbg(const char *msg, const char *why)
{
	static vc wa("__lh_why");
	static vc ma("__lh_msg");
	static vc la("__lh_linenum");
	static vc fa("__lh_filename");
	vc w(why);
	vc m(msg);
	VcDebugNode *dbgn = VcDbgInfo.get();
	if(dbgn == 0)
	{
		la.local_bind(vczero);
		fa.local_bind(vcnil);
	}
	else
	{
		la.local_bind(vc(dbgn->linenum));
		fa.local_bind(vc(dbgn->filename));
	}
	wa.local_bind(w);
	ma.local_bind(m);
	char *d = "lbind(__lh_dbg_ret __lh_debug(<__lh_msg> <__lh_why> <__lh_filename> <__lh_linenum>))";
	vc debug_shell(VC_CVAR, d, strlen(d));
	vc v("__lh_dbg_ret");
	debug_shell.force_eval();
	vc v2 = Vcmap->get(v);
	wa.local_bremove();
	ma.local_bremove();
	la.local_bremove();
	fa.local_bremove();
	v.local_bremove();
	if(v2 != vc("cont") && !Vcmap->backout_in_progress())
		Vcmap->set_dbg_backout();
	CHECK_ANY_BO(1);
	return 0;
}
#endif
	
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
