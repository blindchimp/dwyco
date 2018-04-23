
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCEVAL
#include "vcfunc.h"
#include "vcmap.h"
#include "vcio.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfunc.cpp 1.45 1996/11/17 05:58:41 dwight Stable $";

#ifdef LHPROF
DwSet<vc_func *> vc_func::Func_list;

unsigned long
hash(vc_func * const & a)
{
	return (unsigned long)a;
}
#endif

void
vc_func::init_rest(int v)
{
	if(v & ~(VC_FUNC_VARADIC|VC_FUNC_DONT_EVAL_ARGS|VC_FUNC_CONSTRUCT))
    	oopanic("bogus function style");
	varadic = !!(v & VC_FUNC_VARADIC);
	eval_args = !(v & VC_FUNC_DONT_EVAL_ARGS);
	is_construct = !!(v & VC_FUNC_CONSTRUCT);
#ifdef LHPROF
	call_count = 0;
	total_time = 0;
	total_time2 = 0;
	total_sys_time = 0;
	total_sys_time2 = 0;
	total_real_time = 0;
	total_real_time2 = 0;
	Func_list.add(this);
#endif
}

vc_func::vc_func(const vc& nm, int v)
	: name(nm)
#ifdef LHPROF
	,
	hist_time(20, .050/1000),
	hist_sys_time(20, .050/1000),
	hist_real_time(20, .050/1000)
#endif
{
	init_rest(v);
}

vc_func::vc_func(int v)
	: name(vc("unknown"))
#ifdef LHPROF
	,
	hist_time(20, .050/1000),
	hist_sys_time(20, .050/1000),
	hist_real_time(20, .050/1000)
#endif
{
	init_rest(v);
}

#ifdef LHPROF
vc_func::~vc_func()
{
	Func_list.del(this);
}

#endif

int
vc_func::is_varadic() const {return varadic;}

int
vc_func::must_eval_args() const
{
	return eval_args;
}

vc
vc_func::funmeta() const
{
	USER_BOMB("meta-data not available for that sort of function.", vcnil);
}

vc
vc_func::get_special() const {return name;}

void
vc_func::bomb_func(const char *type) const {
	char s[1000];
	sprintf(s, "can't convert function to %s\n", type);
	USER_BOMB2(s);
}

void
vc_func::bomb_op() const
{
	USER_BOMB2("can't do arithmetic on functions");
}

enum vc_type
vc_func::type() const { return VC_FUNC; }

int
vc_func::operator ==(const vc &v) const {return v.func_eq(*this);}
int
vc_func::operator !=(const vc &v) const {return !v.func_eq(*this);}

int
vc_func::visited()
{
	// this is ok, since we don't traverse past functions anyway
	return FALSE;
}

void
vc_func::printOn(VcIO outputStream) 
{
	outputStream << (varadic ? "func(...)" : "func(fixed)");
}

vc
vc_func::operator()(VCArglist *al) const
{
	do_function_initialize(al);
	do_arg_setup(al);
	vc retval = ((vc_func *)this)->do_function_call(al);
	do_function_finalize(al);
	return retval;
}


void
vc_func::do_function_initialize(VCArglist *) const
{
	if(!is_construct)
		Vcmap->open_ctx();
}

void
vc_func::do_function_finalize(VCArglist *)  const
{
	if(!is_construct)
		Vcmap->close_ctx();
}

void
vc_func::do_arg_setup(VCArglist *) const
{
}

vc
vc_func::do_function_call(VCArglist *, int) 
{
	oopanic("unimp funcall");
	return vcnil;
}

#endif
