
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifdef LHOBJ
//
// implementation of factory definition object
//
#include "vcfac.h"
#include "vcmap.h"
#include "vcobj.h"
#include "dwstr.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfac.cpp 1.45 1997/10/05 17:27:06 dwight Stable $";

const vc vc_factory_def::Basename("base");
const vc vc_factory_def::Thisname("this");
const vc vc_factory_def::Postinitname("__lh_postinit");
const vc vc_factory_def::Preinitname("__lh_preinit");
const vc vc_factory_def::Postdestroyname("__lh_postdestroy");
const vc vc_factory_def::Predestroyname("__lh_predestroy");

#ifdef LHPROF
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

static inline
double
subtimeval(struct timeval& t1, struct timeval& t0)
{
	return (t1.tv_sec + (double)t1.tv_usec / 1000000.0) -
		(t0.tv_sec + (double)t0.tv_usec / 1000000.0);
}
#endif

vc_factory_def::vc_factory_def(const vc& fac_name, vc& args,
	VCArglist *mem_pairs, const vc& base, vc& forwards_list, vc& delegates_list)
{
	int n;
	int i;

	name = fac_name;
	base_expr = base;

	// copy over arguments into bindargs
	if(args.is_nil())
		n = 0;
	else
		n = args.num_elems();
    bindargs = new DwVec<vc>;
	for(i = 0; i < n; ++i)
        (*bindargs).append(args[i]);
		
	// copy in names that are automatically forwarded

	if(forwards_list.is_nil())
		n = 0;
	else
		n = forwards_list.num_elems();
	for(i = 0; i < n; ++i)
	{
		if(forwards_list[i].type() != VC_STRING)
		{
			USER_BOMB2("forwarding member names must be strings");
			/*NOTREACHED*/
		}
		forwards[i] = forwards_list[i];
	}
	
	if(delegates_list.is_nil())
		n = 0;
	else
		n = delegates_list.num_elems();
	for(i = 0; i < n; ++i)
	{
		if(delegates_list[i].type() != VC_STRING)
		{
			USER_BOMB2("delegate member names must be strings");
			/*NOTREACHED*/
		}
		delegates[i] = delegates_list[i];
	}

	// create a map of members with default values
	n = mem_pairs->num_elems();
	member_map = new VC_FACTORY_MAP(n / 2);

	if(n % 2 != 0)
	{
        USER_BOMB2("args to fac def must be even");
		/*NOTREACHED*/
	}
	for(i = 0; i < n; i += 2)
	{
		if((*mem_pairs)[i].type() != VC_STRING)
		{
			USER_BOMB2("member names must be strings");
			/*NOTREACHED*/
		}
		if(member_map->contains((*mem_pairs)[i]))
		{
			VcError << "warning: duplicate member in factory " << (const char *)this->name << "\n";
			//USER_BOMB2("duplicate member name in factory definition");
			/*NOTREACHED*/
		}
		vc expr = (*mem_pairs)[i + 1];
		if(expr.type() == VC_FUNC)
		{
			// set the name for debugging and stuff
			// will have more than just "lamba-function"
			// for a name
			DwString dname;
			dname = (const char *)this->name;
			dname += "[";
			dname += (const char *)((*mem_pairs)[i]);
			dname += "]";
			vc_func *f = (vc_func *)expr.nonono();
			f->name = dname.c_str();
		}
		member_map->add((*mem_pairs)[i], (*mem_pairs)[i + 1]);
	}
}

vc_factory_def::~vc_factory_def()
{
	delete member_map;
}

vc
vc_factory_def::do_function_call(VCArglist *, int) 
{

#ifdef LHPROF
	struct rusage r0;
	getrusage(RUSAGE_SELF, &r0);
	struct timeval t0;
	gettimeofday(&t0, 0);
#endif
	// note: args are set up, and function context is created
	// with all the right bindings.
	//
	// here is where we generate a new object

	VC_FACTORY_MAP_ITER i(member_map);
	VC_FACTORY_MAP *new_map = new VC_FACTORY_MAP(member_map->num_elems());
	
	// generate an uninitialized object 
	// note: assume vc_object doesn't copy in new_map
	vc_object *oret = new vc_object(this, new_map);
	
	vc ret;
	ret.redefine(oret);
	//
	// special hack: in this phase of object creation, we
	// simply create non-member and put it into the
	// factory's function context. this allows
	// subobject to get the proper "this" when they
	// are being initialized.
	//
	int remove_this = 0;
	if(!Vcmap->is_base_init())
	{
		remove_this = 1;
		Vcmap->local_add(Thisname, ret);
	}
	
	// pre-init gets an object context with only "this" in it,
	// and no other members defined, and no base defined.
	
	vc fun_expr;
	if(member_map->find(Preinitname, fun_expr))
	{
		if(fun_expr.type() != VC_CVAR)
		{
			USER_BOMB("bogus preinit?", vcnil);
		}
		Vcmap->open_obj_ctx(oret);
		fun_expr.force_eval();
		Vcmap->close_obj_ctx();
		CHECK_ANY_BO(vcnil);
	}

	// evaluate the base expr
	// the obj context for base evaluation is also empty
	// except for "this"

	vc baseval;
	if(base_expr.type() == VC_CVAR)
	{
		Vcmap->set_base_init(1);
		baseval = base_expr.force_eval();
		Vcmap->set_base_init(0);
		CHECK_ANY_BO(vcnil);
	}
	else
	{
		baseval = base_expr;
	}

	// make new map and copy/eval defaults into the map

	for(; !i.eol(); i.forward())
	{
		DwAssocImp<vc,vc> a = i.get();
		vc name = a.get_key();
		if(name == Postinitname || name == Preinitname ||
			name == Predestroyname || name == Postdestroyname)
			continue;
		vc expr = a.get_value();
		vc val;
		if(expr.type() == VC_CVAR)
		{
			Vcmap->open_obj_ctx(oret);
			val = expr.force_eval();
			Vcmap->close_obj_ctx();
			CHECK_ANY_BO(vcnil);
			if(val.type() == VC_FUNC)
			{
				// set the name for debugging and stuff
				// will have more than just "lamba-function"
				// for a name
				DwString dname;
				dname = (const char *)this->name;
				dname += "[";
				dname += (const char *)a.peek_key();
				dname += "]";
				vc_func *f = (vc_func *)val.nonono();
				f->name = dname.c_str();
			}
		}
		else
			val = expr;
		new_map->add(a.peek_key(), val);
	}
	if(new_map->contains(Basename))
	{
		USER_BOMB("member name \"base\" is reserved", vcnil);
		/*NOTREACHED*/
	}
	new_map->add(Basename, baseval);
	if(new_map->contains(Thisname))
	{
		USER_BOMB("member name \"this\" is reserved", vcnil);
		/*NOTREACHED*/
	}

	++instance_count;       
	//if(Vcmap->ret_in_progress())
	//	ret = Vcmap->retval();

	if(member_map->find(Postinitname, fun_expr))
	{
		// now we can put in a pseudo-member function call to
		// postinit by opening an object context and then
		// evaling the post-init expr. this allows it to have
		// normal access to all the members just created (including
		// this, base, etc.)
		if(fun_expr.type() != VC_CVAR)
		{
			USER_BOMB("bogus postinit?", vcnil);
		}
		Vcmap->open_obj_ctx(oret);
		fun_expr.force_eval();
		Vcmap->close_obj_ctx();
		CHECK_ANY_BO(vcnil);
	}
	
	if(remove_this)
		Vcmap->local_remove(Thisname);
	new_map->add(Thisname, ret);
	// note: "this" is a circular reference, so we tweak the
	// reference count to allow the object to be reclaimed.
	// this is a little bit dicey, because the the circular
	// referent's count will go to -1, which could cause
	// some things to break if they weren't expecting this.
	// this wouldn't be necessary if we used some kinda mark-and-sweep
	// stuff, but see the comments in vcdeflt on that whole thing.
	--oret->ref_count;
	oret->forwards = forwards;
	oret->delegates = delegates;
	oret->base_obj = baseval;

#ifdef LHPROF
	struct rusage r1;
	getrusage(RUSAGE_SELF, &r1);
	struct timeval t1;
	gettimeofday(&t1, 0);

	++call_count;
	
	double n = subtimeval(r1.ru_utime, r0.ru_utime);
	total_time += n;
	total_time2 += n * n;
	hist_time.add_sample(n);
	n = subtimeval(r1.ru_stime, r0.ru_stime);
	total_sys_time += n;
	total_sys_time2 += n * n;
	hist_sys_time.add_sample(n);
	n = subtimeval(t1, t0);
	total_real_time += n;
	total_real_time2 += n * n;
	hist_real_time.add_sample(n);
#endif
	
	return ret;
}
#endif
