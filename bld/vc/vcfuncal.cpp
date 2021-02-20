
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include <string.h>
#include "vcdeflt.h"
#include "vcfuncal.h"
#include "vcmap.h"
#include "dwlista.h"
#include "vcio.h"
void dbg_print_date();
unsigned long vc_funcall::Cache_counter;
#ifndef NO_VCEVAL

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfuncal.cpp 1.50 1998/06/26 02:16:43 dwight Exp $";


#ifdef VCDBG
#include "vcdbg.h"

class VcFuncallDbgNode : public VcDebugNode
{
public:
	VcFuncallDbgNode(const vc_funcall *);

	const vc_funcall *fc;
    vc fun;
	VCArglist *al;
	int argnum;
    vc retval;

    virtual void printOn(VcIO) ;
    virtual void printOnBrief(VcIO);
    virtual int has_brief();

};

VcFuncallDbgNode::VcFuncallDbgNode(const vc_funcall *v)
{
	fc = v;
	al = 0;
	argnum = -1;
	filename = fc->start.filename;
	linenum = fc->start.linenum;
}

int
VcFuncallDbgNode::has_brief()
{
    int r = fc->start.filename.rfind("/");
    if(r == DwString::npos)
        r = fc->start.filename.rfind("\\");
	vc file;
    if(r != DwString::npos)
        file = vc(fc->start.filename.c_str() + r + 1);
	else
        file = vc(fc->start.filename.c_str());
	
	static vc debug("lhdbg.lh");
	static vc userinp("__lh_user_input");
	if(file == debug || file == userinp)
		return 0;
	return 1;
}

void
VcFuncallDbgNode::printOnBrief(VcIOHack &os)
{
	VcDebugNode::printOnBrief(os);
	vc name;
    os << fc->start.filename.c_str() << ":" <<
		 fc->start.linenum << ", ";
	if(info == vc("Finding function"))
	{
		os << "(fundef search) ";
	}
	else
	{
		if(fun.type() != VC_FUNC && fun.type() != VC_MEMFUN)
			name = vc("<<erroneous>>");
		else
			name = fun.get_special();
        if(info == vc("Processing arguments"))
		{
			os << (const char *)name ;
			if(argnum == -1)
				os << "() ";
			else
			{
				os << "(";
				int i;
				for(i = 0; i <= argnum - 1; ++i)
					os << "+ ";
				os << "! ";
				for(++i; i < fc->arglist.num_elems(); ++i)
					os << "- ";
				os << ")";
			}
		}
		else if(info == vc("Calling function"))
		{
			os << (const char *)name << "(";
			for(int i = 0; i < al->num_elems(); ++i)
			{
				if((*al)[i].is_atomic())
				{
					(*al)[i].print_top(os);
					os << " ";
				}
				else
				{
					os << "<<big>> ";
				}
			}
			os << ")";
		}
		else if(info == vc("After function return"))
		{
			os << "returning from " << (const char *)name << "(";
			for(int i = 0; i < al->num_elems(); ++i)
			{
				if((*al)[i].is_atomic())
				{
					(*al)[i].print_top(os);
					os << " ";
				}
				else
				{
					os << "<<big>> ";
				}
			}
			os << ")";
			os << "returned \""; 
			if(retval.is_atomic())
				retval.print_top(os);
			else
				os << "<<big>>";
			os << "\"";
		}
		else
		{
			os << "<<unknown function call state>>\n";
		}
    }
    os << "\n";
}

void
VcFuncallDbgNode::printOn(VcIO os) 
{
	VcDebugNode::printOn(os);
	vc name;
    os << "---Funcall near line " << fc->start.filename.c_str() << ":" <<
		 fc->start.linenum << ", ";
	if(info == vc("Finding function"))
	{
		os << "Searching for function definition\n";
	}
	else
	{
		if(fun.type() != VC_FUNC && fun.type() != VC_MEMFUN)
			name = vc("<<erroneous>>");
		else
			name = fun.get_special();
        if(info == vc("Processing arguments"))
		{
			if(argnum == -1)
				os << "No arguments for function " << (const char *)name << "\n";
			else
				os << "Processing argument #" << (int)argnum << " for function " << (const char *)name << "\n";
		}
		else if(info == vc("Calling function"))
		{
			os << "Called " << (const char *)name << "(";
			for(int i = 0; i < al->num_elems(); ++i)
			{
				(*al)[i].print_top(os);
				os << " ";
			}
			os << ")\n";
		}
		else if(info == vc("After function return"))
		{
			os << "Called " << (const char *)name << "(";
			for(int i = 0; i < al->num_elems(); ++i)
			{
				(*al)[i].print_top(os);
				os << " ";
			}
			os << ")\n";
			os << "Which returned \""; 
			retval.print_top(os);
			os << "\"\n";
		}
		else
		{
			os << "<<unknown function call state>>\n";
		}
    }
    os << "\n";
}

#endif

vc_funcall::vc_funcall(const vc& v, const VCList& vl,
	 const vc_cvar_src_coord& s, const vc_cvar_src_coord& e)
	 : start(s), end(e), func(v), cached_fun(new vc),
 cached_when(new unsigned long)
{
	vc arg;
    VCListIter i(&vl);
	dwlista_foreach_iter(i, arg, vl)
		arglist.append(arg);

	*cached_when = 0;
	is_string = 0;
}

vc_funcall::vc_funcall(const vc_funcall& v)
	: arglist(v.arglist), func(v.func), cached_fun(new vc), 
 cached_when(new unsigned long)
{

	*cached_when = *v.cached_when;
	is_string = v.is_string;
}

vc_funcall::~vc_funcall()
{
	delete cached_fun;
	delete cached_when;

}


void
vc_funcall::flush_cache()
{
	*cached_fun = vcnil;
}


vc
vc_funcall::eval() const
{

#ifdef VCDBG
	VcFuncallDbgNode _dbg(this);
#define dbg(x) _dbg.x
	dbg(info) = "Finding function";
#endif

	vc f;
	int vetted = 0;
	if(is_string || func.type() == VC_STRING)
	{
        is_string = 1;
		// convenience: if func is a string, then do the mapping
		// instead of causing the user to write <f>(...) all the time.
		// note: foo() doesn't have any other meaning anyway, so this
		// is no big deal.
		//
#ifdef FUNCACHE
		if(*cached_when < Cache_counter || (*cached_fun).is_nil())
		{
			*cached_fun = Vcmap->get(func);
			*cached_when = Cache_counter;
			f = *cached_fun;
			// can't cache member function look ups this way
			if(f.type() == VC_MEMFUN)
			{
				*cached_when = 0;
				*cached_fun = vcnil;
				vetted = 1;
			}
			else if(f.type() == VC_FUNC)
				vetted = 1;
			else
			{
				*cached_when = 0;
				*cached_fun = vcnil;
				vetted = 0;
			}
		}
		else
		{
			f = *cached_fun;
			vetted = 1;
		}
#else
		f = Vcmap->get(func);
#endif
	}
	else
	{
		// no caching for computed functions
		f = func.eval();
		CHECK_ANY_BO(vcnil);
	}
	// check this early, so we can stop execution
	// now, instead of waiting for the actual function
	// call later (after arg evaluation)
	//
	if(!vetted && f.type() != VC_FUNC && f.type() != VC_MEMFUN)
	{
		*cached_fun = vcnil;
        VcIOHackStr o;
		if(func.type() == VC_STRING)
            o << "can't find function named \"" << (const char *)func << "\"";
		else
            o << "attempt to call non-function";
        o << '\0';
        USER_BOMB(o.ref_str(), vcnil);
	}

	int n = arglist.num_elems();

    //VCArglist al(n, 0, 1, 8, 0, 1);
    VCArglist al;
    al.set_size(n);

#ifdef VCDBG
	dbg(fun) = f;
	dbg(al) = &al;
	dbg(info) = "Processing arguments";
#endif

	if(!f.must_eval_args())
	{
		for(int i = 0; i < n; ++i)
		{
            al.append(arglist[i]);
		}
	}
	else
	{
		for(int i = 0; i < n; ++i)
		{
#ifdef VCDBG
			dbg(argnum) = i;
#endif
            al.append(arglist[i].eval());
			if(Vcmap->dbg_backout_in_progress())
			{
				dbg_print(f, i);
				return vcnil;
			}
			CHECK_ANY_BO(vcnil);
		}
	}

#ifdef VCDBG
	dbg(info) = "Calling function";
#endif

	vc ret = f(&al);

#ifdef VCDBG
	dbg(info) = "After function return";
	dbg(retval) = ret;
#endif

	if(Vcmap->dbg_backout_in_progress())
	{
		dbg_print(f, &al);
		return vcnil;
	}
	return ret;
}

void
vc_funcall::stringrep(VcIO o) const
{
	int nargs = arglist.num_elems();

	func.stringrep(o);
	o << "(";
	for(int i = 0; i < nargs; ++i)
	{
		arglist[i].stringrep(o);
		o << " ";
	}
	o << ")";
}

void
vc_funcall::dbg_print(const vc& fun, int arg) const 
{
	vc name;

	dbg_print_date();
	if(fun.type() != VC_FUNC && fun.type() != VC_MEMFUN)
		name = vc("<<erroneous>>");
	else
		name = fun.get_special();

	VcError << "Processing arg " << arg << " for function " << (const char *)name << "\n";
}

void
vc_funcall::dbg_print(const vc& fun, VCArglist *al) const 
{
	vc name;

	dbg_print_date();
	if(fun.type() != VC_FUNC && fun.type() != VC_MEMFUN)
		name = vc("<<erroneous>>");
	else
		name = fun.get_special();

	VcError << "Called ";
	name.print(VcError);
	VcError << "(";
	for(int i = 0; i < al->num_elems(); ++i)
	{
		(*al)[i].print_top(VcError);
		VcError << " ";
	}
	VcError << ")\n";
} 
#endif
void
vc_funcall::flush_all_cache()
{
	if(++Cache_counter == 0)
		oopanic("cache counter overflow"); // fix this
}
