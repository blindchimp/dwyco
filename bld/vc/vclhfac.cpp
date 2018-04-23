
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifdef LHOBJ
#include "vcfac.h"
#include "vcmap.h"
#include "vclhfac.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vclhfac.cpp 1.44 1996/11/17 05:59:37 dwight Stable $";

// functions for object factories in LH

//
// make a factory for an LH object
// the arguments should be pre-evaled.
//
static vc
make_factory_def(VCArglist *a, vc& name_out)
{
	//
	// args to make a factory are:
	// factory name
	// vector of arguments used when object is created (or nil).
	// base expr (or nil)
	// vector of delegating names (or nil)
	// vector of forwarding names (or nil)
	// pairs of the form:
	//		string expr

	int nargs = a->num_elems();

	if(nargs <= 3)
	{
		USER_BOMB("make_factory must have at least 4 args (name, args, base and forwards)", vcnil);
		/*NOTREACHED*/
	}

	vc name = (*a)[0];
	vc args = (*a)[1];
	if(!args.is_nil() && args.type() != VC_VECTOR)
	{
		USER_BOMB("list of args must be a vector in make_factory", vcnil);
		/*NOTREACHED*/
	}
	vc base = (*a)[2];
	vc delegates = (*a)[3];
	if(!delegates.is_nil() && delegates.type() != VC_VECTOR)
	{
		USER_BOMB("list of delegate names in make_factory must be a vector()", vcnil);
		/*NOTREACHED*/
	}
	vc forwards = (*a)[4];
	if(!forwards.is_nil() && forwards.type() != VC_VECTOR)
	{
		USER_BOMB("list of forwarding names in make_factory must be a vector()", vcnil);
		/*NOTREACHED*/
	}
	a->del(0, 5);

	if(a->num_elems() % 2 != 0)
	{
		USER_BOMB("each member must have a default expression", vcnil);
		/*NOTREACHED*/
	}

	vc_factory_def *vfd = new vc_factory_def(name, args, a, base, forwards, delegates);
	vc facdef;
	facdef.redefine(vfd);
	name_out = name;
	return facdef;
}

vc
vclh_lmake_factory(VCArglist *a)
{
	vc name;
	vc f = make_factory_def(a, name);
	name.local_bind(f);
	return vcnil;
}

vc
vclh_gmake_factory(VCArglist *a)
{
	vc name;
	vc f = make_factory_def(a, name);
	name.global_bind(f);
	return vcnil;
}

vc
vclh_make_factory(VCArglist *a)
{
	vc name;
	vc f = make_factory_def(a, name);
	name.bind(f);
	return vcnil;
}
#endif
