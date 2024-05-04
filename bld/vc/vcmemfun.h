
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VC_MEMFUN_H
#define VC_MEMFUN_H

// class that encapsulates an object member function.
// this is used in order to allow member functions
// to have special calling semantics, and also to allow
// member functions to be used as (at least) second-class
// LH objects.

// $Header: g:/dwight/repo/vc/rcs/vcmemfun.h 1.44 1996/11/17 05:59:38 dwight Stable $

#include "vcfundef.h"
#ifdef LHOBJ
class vc_object;

class vc_memberfun : public vc_fundef
{
friend class vc_object;
public:
	vc_memberfun(vc_object *, const vc& fun);
	~vc_memberfun();

	enum vc_type type() const ;
	vc get_special() const;


#ifdef VCDBG
	virtual void set_break_on(int);
	virtual void clear_break_on(int);
	virtual int break_on(int) const;
#endif

private:
	void set_obj(vc_object *);
	
#if 0
	virtual void do_function_finalize(VCArglist *) const;
	virtual void do_function_initialize(VCArglist *) const;
	virtual vc do_function_call(VCArglist *) const;
#endif
	virtual vc operator()(VCArglist *) const;
#if 0
	virtual vc operator()(vc v1) const {return vc_fundef::operator()(v1);}
	virtual vc operator()(vc v1, vc v2) const {return vc_fundef::operator()(v1, v2);}
	virtual vc operator()(vc v1, vc v2, vc v3) const {return vc_fundef::operator()(v1, v2, v3);}
	virtual vc operator()(vc v1, vc v2, vc v3, vc v4) const {return vc_fundef::operator()(v1, v2, v3, v4);}
	virtual vc operator()(void) const {return vc_fundef::operator()();}
	//virtual vc operator()(void *p) const {return vc_fundef::operator()(p);}
#endif
	
	void printOn(VcIO outputStream) ;

	vc obj;	// object function associated with
	vc fun;	// function to eval when called
};



#endif
#endif
