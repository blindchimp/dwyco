
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifdef LHOBJ
#include "vcobj.h"
#include "vcmap.h"
#include "vcmemfun.h"
#include "dwstr.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcmemfun.cpp 1.44 1996/11/17 05:59:32 dwight Stable $";

vc_memberfun::vc_memberfun(vc_object *vcobj, const vc& fundef)
	: fun(fundef)
{
	obj.attach(vcobj);
}

vc_memberfun::~vc_memberfun()
{
}

void
vc_memberfun::set_obj(vc_object *vcobj)
{
	obj.attach(vcobj);
}

enum vc_type
vc_memberfun::type() const 
{
	return VC_MEMFUN;
}

vc
vc_memberfun::get_special() const
{
	char s[100];
    snprintf(s, sizeof(s), "%p ", obj.rep);
	DwString a(s);
	a += (const char *)fun.get_special();
	vc ret(a.c_str());
	return ret;
}

// gross: this forwards out a call to the member function
// with interspersed object context manipulations.
//
vc
vc_memberfun::operator()(VCArglist *a) const
{
	vc_fundef *vfd = fun.get_fundef();
	vfd->do_function_initialize(a);
	Vcmap->open_obj_ctx(obj.get_obj());
	vfd->do_arg_setup(a);
#ifdef VCDBG
	// if the object has a break tag, that means we only
	// want to break if the member function also has a
	// break tag.  this allows breaking on entry to
	// a particular object's member (instead of
	// any object's member)
	
	vc_object *o = obj.get_obj();
	vc retval;
	if(break_on(BREAK_MEMCALL))
	{
		if(drop_to_dbg("memberfun call break", "membreak"))
			retval = vcnil;
		else
			retval = vfd->do_function_call(a, 1);
	}
	else
		retval = vfd->do_function_call(a);
#else
	vc retval = vfd->do_function_call(a);
#endif
	Vcmap->close_obj_ctx();
	vfd->do_function_finalize(a);
	return retval;
}

void
vc_memberfun::printOn(VcIO outputStream) 
{
	outputStream << "member-";
	vc_fundef::printOn(outputStream);
}


#ifdef VCDBG
// this is a bit strange, but it forwards the calls out
// to the components of the member function (where they are
// remembered) since member functions don't really exist
// like fundefs
void
vc_memberfun::set_break_on(int m)
{
	// when you break on a member function in this
	// way, it means you want to break when the specific
	// member is called on this object.
	obj.set_break_on(m);
	fun.set_break_on(m);
}

void
vc_memberfun::clear_break_on(int m)
{
	obj.clear_break_on(m);
	fun.clear_break_on(m);
}

int
vc_memberfun::break_on(int m) const
{
	return obj.break_on(m) & fun.break_on(m);
}
#endif
#endif
