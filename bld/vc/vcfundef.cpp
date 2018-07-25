
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCEVAL
#include "vcdeflt.h"
#include "vcfundef.h"
#include "vcmap.h"
#include "dwvec.h"
#include "vcio.h"
#include "dwqmap3.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfundef.cpp 1.47 1997/10/05 17:27:06 dwight Stable $";

vc_fundef::vc_fundef(const char *name, VCArglist *a, int sty, int decrypt)
    :  vc_func(vc(name), sty)
{
	int nargs = a->num_elems();
    bindargs = new DwVec<vc>;
    primed_maps.append(new argstate);
    lctx = &primed_maps[0]->fctx;
    recurse = 0;

	// if a vector is passed as the first argument,
	// we assume its is a list of arguments
	// to be broken-out
	if((*a)[0].type() == VC_VECTOR)
	{
		if(nargs != 2)
		{
			USER_BOMB2("must have only 2 arguments when defining function with vector of args");
		}
		int nvargs = (*a)[0].num_elems();
		for(int i = 0; i < nvargs; ++i)
		{
			(*bindargs)[i] = (*a)[0][i];
		}
	}
	else
	{
		for(int i = 0; i < nargs - 1; ++i)
			(*bindargs)[i] = (*a)[i];
    }

	fundef = (*a)[nargs - 1];

	if(fundef.is_atomic())
	{
		if(fundef.type() == VC_STRING)
		{
			fundef = vc(VC_CVAR, fundef, decrypt ? -fundef.len() : fundef.len()); // try to compile it
		}
		else
			fundef = vc(VC_CVAR, fundef); // try to compile it
	}
	// otherwise, assume it was already compiled
}

vc_fundef::vc_fundef(int sty) : vc_func(vc("<<internal>>"), sty)
{
	bindargs = 0;
}

vc_fundef::vc_fundef(vc name, int sty) :
    vc_func(name, sty)
{
    bindargs = 0;
}

int
vc_fundef::func_eq(const vc& v) const
{
	return fundef == ((const vc_fundef &)v).fundef &&
		*bindargs == *((const vc_fundef &)v).bindargs;
}

void
vc_fundef::stringrep(VcIO o) const
{
	if(name == vc("lambda-function"))
		o << "lambda(";
    else
    {
		o << "compile(";
		name.stringrep(o);
	}
	int nargs = bindargs->num_elems();
	for(int i = 0; i < nargs; ++i)
	{
		(*bindargs)[i].stringrep(o);
        o << " ";
	}
	// when something is compiled, a level of quoting is
	// stripped off. We put one back on here...
	// this isn't right... need to fix this somehow...
	//
	//o << "`";
	fundef.stringrep(o);
	//o << "'";
	o << ")";
}


vc_fundef::~vc_fundef()
{
	delete bindargs;
    for(int i = primed_maps.num_elems() - 1; i >= 0; --i)
    {
        delete primed_maps[i];
    }
}

//
// eval a fundef.
// it is assumed that all of the argument bindings
// and so on are set up before this is done.
// This, in essence, does the same thing as
// evaluating an expression. Only at this point,
// the user has somehow gotten hold of this fundef object
// instead of a cvar object. this is a very
// primitive op, and should only be done in
// cases where it is absolutely necessary.
//
//
vc
vc_fundef::eval() const
{
	return fundef.force_eval();
}

void
vc_fundef::do_function_initialize(VCArglist *) const
{
    if(!is_construct)
    {
        if(primed_maps[recurse] == 0)
        {
            primed_maps[recurse] = new argstate;
        }
        lctx = &primed_maps[recurse]->fctx;
        Vcmap->open_ctx(lctx);
    }
}

void
vc_fundef::do_function_finalize(VCArglist *)  const
{
    if(!is_construct)
    {
        Vcmap->close_ctx();
        lctx->reset();
        DwQMapLazyC<vc,vc,32> *m = dynamic_cast<DwQMapLazyC<vc,vc,32> *>(lctx->map);
        if(m)
        {
            if(m->restore() == 0)
            {
                delete primed_maps[recurse - 1];
                primed_maps[recurse - 1] = new argstate;

            }
        }
        --recurse;
        lctx = &primed_maps[recurse]->fctx;
        if(recurse != 0)
            primed_maps[primed_maps.num_elems() - 1] = 0;
    }
}
//
// set up arguments for a user-defined function call.
// note that arguments are assumed to be evaled, if
// that is what the function definition calls for.
// the reason we leave arg evaluation with the caller
// is that if makes it easier to deal with debugging
// information, and the args really are supposed to be
// evaluated in the context of the caller, not the
// callee.
//
void
vc_fundef::do_arg_setup(VCArglist *a) const
{
	int n_formal_args = bindargs->num_elems();
	int n_call_args = a->num_elems();
	if(varadic && n_call_args < n_formal_args)
	{
		VcError << "warning: varadic " << (is_construct ? "construct" : "function")
			<<  " \""; 
        vc a = name;
		a.print(VcError);
		VcError << "\" called with fewer arguments than definition, "
            "unspec'ed args lbinded to nil\n";
	}
	if(!varadic && n_call_args != n_formal_args)
	{
		VcError << "warning: non-varadic " <<
			(is_construct ? "construct" : "function") <<  " \"";
        vc a = name;
		a.print(VcError);
		VcError << "\" called with " <<
			((n_call_args < n_formal_args) ?
				"fewer args than expected, unspec'ed args lbinded to nil." :
				"more args than expected, extra args ignored.") << "\n";
	}

    argstate *as = primed_maps[recurse];
    if(!as->primed)
    {
        for(int i = 0; i < n_formal_args; ++i)
        {
            lctx->add((*bindargs)[i], (i >= n_call_args) ? vcnil : (*a)[i], &(as->wps[i]));
            //Vcmap->local_add((*bindargs)[i], (i >= n_call_args) ? vcnil : (*a)[i]);
        }
        as->primed = 1;
        DwQMapLazyC<vc,vc,32> *m = dynamic_cast<DwQMapLazyC<vc,vc,32> *>(lctx->map);
        if(m)
            m->snapshot();
    }
    else
    {
        for(int i = 0; i < n_formal_args; ++i)
        {
            if(as->wps[i])
                *(as->wps[i]) = (i >= n_call_args) ? vcnil : (*a)[i];
        }
    }

	if(varadic)
	{
		// bundle up the trailing args into a special variable
		// and lbind this into the function context.
		vc trailing(VC_VECTOR);
		int i;
		int j;
		for(i = n_formal_args, j = 0; i < n_call_args; ++i, ++j)
			trailing[j] = (*a)[i];
        Vcmap->local_add("__lh_varargs", trailing);
	}
}

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

vc
vc_fundef::do_function_call(VCArglist *, int suppress_break) 
{

	// note: args are set up,
	// don't send out via operator() in vc_cvar,
	// that's only good for sending things out to C++ env.
	//  vc ret = fundef(bindargs);  <--- no good
	// (args are passed via bindings in local environment)
#ifdef VCDBG
	if(!suppress_break && break_on(BREAK_CALL))
	{
		if(drop_to_dbg("function call break", "funbreak"))
			return vcnil;
	}
#endif
#ifdef LHPROF
	struct rusage r0;
	getrusage(RUSAGE_SELF, &r0);
	struct timeval t0;
	gettimeofday(&t0, 0);
#endif
    if(!is_construct)
        ++recurse;
	vc ret = fundef.force_eval();
	if(Vcmap->ret_in_progress())
		ret = Vcmap->retval();
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


vc
vc_fundef::funmeta() const
{
	vc meta(VC_VECTOR);

	int nargs = bindargs->num_elems();
	for(int i = 0; i < nargs; ++i)
		meta.append((*bindargs)[i]);
	return meta;
}

#endif
