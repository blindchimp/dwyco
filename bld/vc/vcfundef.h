
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCFUNDEF_H
#define VCFUNDEF_H
// $Header: g:/dwight/repo/vc/rcs/vcfundef.h 1.46 1997/10/05 17:27:06 dwight Stable $

#include "vc.h"
#include "vcfunc.h"
#include "dwvecp.h"
#include "vcfunctx.h"

//
// class encapsulating a function definition and call
// objects of this type are used to represent internal
// function calls (as opposed to the other function
// classes which are used to represent calls to functions
// in the C++ world.)
//
class vc_fundef : public vc_func
{
friend class vc;
friend class vc_cvar;
friend class vc_funcall;
friend class vc_memberfun;
friend vc dofunbuild(const char *, VCArglist *, int, int);

protected:
        DwVec<vc> *bindargs;
        mutable functx lctx;
        mutable int recurse;
        mutable int primed;
        mutable DwVecP<vc> wps;

	
	vc_fundef(int = VC_FUNC_NORMAL); // used for special function definitions (factories)
    vc_fundef(vc name, int sty);
	~vc_fundef();
    void do_function_initialize(VCArglist *) const;
    void do_function_finalize(VCArglist *)  const;

	virtual void do_arg_setup(VCArglist *) const;
	virtual vc do_function_call(VCArglist *, int suppress_break = 0) ;

private:
	vc fundef;
	vc_fundef(const char *name, VCArglist *al, int = VC_FUNC_NORMAL, int decrypt = 0);

	vc_fundef *get_fundef() const {return (vc_fundef *)this;}

	vc eval() const; 
	virtual vc_default *do_copy() const {oopanic("say what?"); return 0;}
	virtual vc funmeta() const;

public:
	virtual void stringrep(VcIO) const;
	//vc operator()(VCArglist *) const;

	int func_eq(const vc& v) const;
	
};
#endif
