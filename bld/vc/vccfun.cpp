
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCEVAL
#include "vccfun.h"
#include "dwvec.h"
#include "vcdbg.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vccfun.cpp 1.47 1998/06/22 03:56:26 dwight Exp $";

#define ctor_vcfuncp(t) \
vc::vc(t p, const char *name, const char *impl_name, int style, VCTRANSFUNCP tfp) \
{ \
    rep = new vc_cfunc(p, name, impl_name, style, tfp); \
}
ctor_vcfuncp(VCFUNCP0)
ctor_vcfuncp(VCFUNCP1)
ctor_vcfuncp(VCFUNCP2)
ctor_vcfuncp(VCFUNCP3)
ctor_vcfuncp(VCFUNCP4)
ctor_vcfuncp(VCFUNCP5)
//ctor_vcfuncp(VCFUNCPVP)
ctor_vcfuncp(VCFUNCPv)
#undef ctor_vcfuncp

void
vc_cfunc::do_arg_setup(VCArglist *a) const
{
	int n_formal_args = nargs;
	int n_call_args = a->num_elems();
	if(varadic && n_call_args < n_formal_args)
	{
		VcError << "warning: builtin varadic " << (is_construct ? "construct" : "function")
			<<  "\"" << (const char *)name << "\" called with fewer arguments than definition\n";
	}
	if(!varadic && n_call_args != n_formal_args)
	{
		VcError << "warning: builtin non-varadic " <<
			(is_construct ? "construct" : "function") <<  " \"" << (const char *)name << "\" called with " <<
			((n_call_args < n_formal_args) ?
				"fewer args than expected, unspec'ed args treated as nil." :
				"more args and expected, extra args ignored.") << "\n";

    }
    // note: for varadic funcs, we leave it up to the implementation of the
    // functions to emit warnings/errors for arg count.
    // alternative would be to specify min args instead
    if(!varadic)
    {
        while(n_call_args < n_formal_args)
        {
            a->append(vcnil);
            ++n_call_args;
        }
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
vc_cfunc::do_function_call(VCArglist *a, int) const
{
#ifdef VCDBG
	if(break_on(BREAK_CALL))
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
	vc ret;
	if(varadic)
		ret = (*funcpv)(a);
	else
	{

		switch(dwmax(a->num_elems(), (long)nargs))
		{
		case 0:
			ret =  (*funcp0)();
			break;
		case 1:
			ret =  (*funcp1)((*a)[0]);
			break;
		case 2:
			ret =  (*funcp2)((*a)[0], (*a)[1]);
			break;
		case 3:
			ret =  (*funcp3)((*a)[0], (*a)[1], (*a)[2]);
			break;
		case 4:
			ret =  (*funcp4)((*a)[0], (*a)[1], (*a)[2], (*a)[3]);
			break;
		case 5:
			ret =  (*funcp5)((*a)[0], (*a)[1], (*a)[2], (*a)[3], (*a)[4]);
			break;
		default:
			oopanic("arg overflow");
			/*NOTREACHED*/
		}
	}
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

vcy
vc_cfunc::internal_call(VCArgHolder *al) const
{
    // this doesn't make a lot of sense, but just for testing, we'll do this here
    VCArglist aa;
    VCArglist *a = &aa;
    int n = al->num_elems();
    aa.set_size(n);
    for(int i = 0; i < n; ++i)
    {
        aa.append(vc((*al)[i]));
    }
    if(varadic)
    {
        return (*funcpv)(a);
    }
    vc ret;
    switch(dwmax(a->num_elems(), (long)nargs))
    {
    case 0:
        ret =  (*funcp0)();
        break;
    case 1:
        ret =  (*funcp1)((*a)[0]);
        break;
    case 2:
        ret =  (*funcp2)((*a)[0], (*a)[1]);
        break;
    case 3:
        ret =  (*funcp3)((*a)[0], (*a)[1], (*a)[2]);
        break;
    case 4:
        ret =  (*funcp4)((*a)[0], (*a)[1], (*a)[2], (*a)[3]);
        break;
    case 5:
        ret =  (*funcp5)((*a)[0], (*a)[1], (*a)[2], (*a)[3], (*a)[4]);
        break;
    default:
        oopanic("arg overflow");
        /*NOTREACHED*/
    }
    return ret;
}

#if 0
// note: this was a quick hack to output all the decls
// for bound c funcs. i didn't want to have to type them all
// by hand...
#include "dwstr.h"
static
const char *
genargs(int n)
{
	static DwString a;
	a = "";
	for(int i = 0; i < n; ++i)
	{
		a += "vc";
		if(i != n - 1)
			a += ", ";
	}
	return a.c_str();
}
#define ctor(t,n, na) \
    vc_cfunc::vc_cfunc(t##n p, const char *name, const char *impl_fun, int style, VCTRANSFUNCP tfp) \
    : vc_func(vc(name), style) {funcp##n = p;nargs = na; transfunc = tfp; impl_name = impl_fun; printf("vc %s(%s);\n", impl_name, genargs(na));}
vc_cfunc::vc_cfunc(VCFUNCPv p, const char *name, const char *impl_fun, int style, VCTRANSFUNCP tfp)
    : vc_func(vc(name), style | VC_FUNC_VARADIC) {funcpv = p; nargs = -1; transfunc = tfp; impl_name = impl_fun; printf("vc %s(VCArglist *);\n", impl_name);}

#endif

#define ctor(t,n, na) \
    vc_cfunc::vc_cfunc(t##n p, const char *cname, const char *impl_fun, int style, VCTRANSFUNCP tfp) \
    : vc_func(vc(cname), style) {funcp##n = p;nargs = na; transfunc = tfp; impl_name = impl_fun; }
ctor(VCFUNCP,0,0)
ctor(VCFUNCP,1,1)
ctor(VCFUNCP,2,2)
ctor(VCFUNCP,3,3)
ctor(VCFUNCP,4,4)
ctor(VCFUNCP,5,5)
#undef ctor

//vc_cfunc::vc_cfunc(VCFUNCPVP p, const char *name, const char *impl_fun, int style, VCTRANSFUNCP tfp)
//     : vc_func(vc(name), style) {funcp_vp = p; nargs = 1; transfunc = tfp;}
// varadic function
vc_cfunc::vc_cfunc(VCFUNCPv p, const char *cname, const char *impl_fun, int style, VCTRANSFUNCP tfp)
    : vc_func(vc(cname), style | VC_FUNC_VARADIC) {funcpv = p; nargs = -1; transfunc = tfp; impl_name = impl_fun; }

// the following is a bit weird, but it is necessary
// to ensure safety. even though we are in the "non-atomic"
// part of the class hierarchy, we call these objects atomic
// to avoid situations where they might be evaluated. 
// this is safe, because there are no LH/VC constructs
// used at this point in the hierarchy, and there is no
// way to get into what is inside one of these objects anyway from
// LH or VC (they are "built-in"). also, there are no
// possible circular references and these can't be serialized...
// 

int
vc_cfunc::is_atomic() const
{
	return TRUE;
}

vc
vc_cfunc::eval() const
{
	oopanic("attempt to eval cfunc");
	/*NOTREACHED*/
	return vcnil;
}

vc_default *
vc_cfunc::do_copy() const {
	oopanic("can't copy cfun");
	return 0;
}

vc
vc_cfunc::operator()() const {return (*funcp0)();}
vc
vc_cfunc::operator()(vc v0) const {return (*funcp1)(v0);}
vc
vc_cfunc::operator()(vc v0, vc v1) const {return (*funcp2)(v0, v1);}
vc
vc_cfunc::operator()(vc v0, vc v1, vc v2) const {return (*funcp3)(v0, v1, v2);}
vc
vc_cfunc::operator()(vc v0, vc v1, vc v2, vc v3) const {return (*funcp4)(v0, v1, v2, v3);}
//vc
//vc_cfunc::operator()(void *p) const {return (*funcp_vp)(p);}

int
vc_cfunc::func_eq(const vc& v) const { return funcp0 == ((const vc_cfunc&)v).funcp0; }
hashValueType
vc_cfunc::hashValue() const {return (unsigned long)funcp0;}
#endif
