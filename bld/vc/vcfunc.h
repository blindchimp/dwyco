
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCFUNC_H
#define VCFUNC_H
// $Header: g:/dwight/repo/vc/rcs/vcfunc.h 1.46 1997/10/05 17:27:06 dwight Stable $

#include "vc.h"
#include "vccomp.h"

// This top level class simply provides a place to hang a lot of
// common error functionality. The interesting stuff starts in the
// classes that are children of this class.

#include "vcfext.h"
#ifdef LHPROF
#include "dwset.h"
#include "dwhist2.h"
#endif
class vc_func : public vc_composite
{
friend class vc_objfunc;
friend class vc_cfunc;
friend class vc_funcall;
friend class vc_fundef;
friend class vc_memberfun;
friend class vc_obj;
friend class vc_factory_def;
friend class vc_trans_fundef;
friend vc dump_flat_profile();

protected:
	vc name;	// function name, only for printing and debugging
	int varadic;
	int eval_args;
	int is_construct;

	virtual void do_function_initialize(VCArglist *) const;
	virtual void do_arg_setup(VCArglist *) const;
	virtual vc do_function_call(VCArglist *, int = 0) ;
	virtual void do_function_finalize(VCArglist *) const;

private:
	void init_rest(int);
	vc_func(const vc& nm, int v = VC_FUNC_NORMAL);
	vc_func(int v = VC_FUNC_NORMAL);
#ifdef LHPROF
	virtual ~vc_func();
#endif

	vc eval() const = 0;
	virtual int is_varadic() const;
	virtual int must_eval_args() const;

	virtual vc funmeta() const;
	virtual vc get_special() const;
	
public:
	void bomb_func(const char *type) const;
	void bomb_op() const;

	virtual vc operator()(VCArglist *) const;
#if 0
	virtual vc operator()(vc v1) const {return vc_composite::operator()(v1);}
	virtual vc operator()(vc v1, vc v2) const {return vc_composite::operator()(v1, v2);}
	virtual vc operator()(vc v1, vc v2, vc v3) const {return vc_composite::operator()(v1, v2, v3);}
	virtual vc operator()(vc v1, vc v2, vc v3, vc v4) const {return vc_composite::operator()(v1, v2, v3, v4);}
	virtual vc operator()(void) const {return vc_composite::operator()();}
	//virtual vc operator()(void *p) const {return vc_composite::operator()(p);}
#endif
	enum vc_type type() const ;
	virtual int operator ==(const vc &v) const ;
	virtual int operator !=(const vc &v) const ;

	void printOn(VcIO outputStream) ;
	int visited();
#ifdef LHPROF
	static DwSet<vc_func *> Func_list;
        long call_count;
	double total_time;
	double total_time2;
	double total_sys_time;
	double total_sys_time2;
	double total_real_time;
	double total_real_time2;
	// 100 bins, 1 second max -> 10 ms per bin
	// 20 bins, .050 sec max -> 2.5ms per bin
	DwHistogram hist_time;
	DwHistogram hist_sys_time;
	DwHistogram hist_real_time;
#endif
};
#endif
