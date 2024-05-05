
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCCFUN_H
#define VCCFUN_H
// $Header: g:/dwight/repo/vc/rcs/vccfun.h 1.47 1998/06/22 03:56:28 dwight Exp $

#include "vc.h"
#include "vcfunc.h"

//
// The function is a simple C function pointer.
//

class  vc_cfunc : public vc_func
{
friend class vc;
private:
	union {
	VCFUNCP0 funcp0;
	VCFUNCP1 funcp1;
	VCFUNCP2 funcp2;
	VCFUNCP3 funcp3;
	VCFUNCP4 funcp4;
	VCFUNCP5 funcp5;
	VCFUNCPv funcpv;
	};
	int nargs;
	VCTRANSFUNCP transfunc;
	const char *impl_name;
	
#define ctor_decl(t,n) \
    vc_cfunc(t##n p, const char *name, const char *impl_fun, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
ctor_decl(VCFUNCP,0)
ctor_decl(VCFUNCP,1)
ctor_decl(VCFUNCP,2)
ctor_decl(VCFUNCP,3)
ctor_decl(VCFUNCP,4)
ctor_decl(VCFUNCP,5)
#undef ctor

	// varadic function
    vc_cfunc(VCFUNCPv p, const char *name, const char *impl_fun, int style = VC_FUNC_NORMAL|VC_FUNC_VARADIC, VCTRANSFUNCP = 0);

	vc eval() const ;

	virtual vc_default *do_copy() const ;
	virtual vc do_function_call(VCArglist *, int = 0) ;
	virtual void do_arg_setup(VCArglist *) const;

public:
#if 0
	vc operator()() const ;
	vc operator()(vc v0) const ;
	vc operator()(vc v0, vc v1) const ;
	vc operator()(vc v0, vc v1, vc v2) const ;
	vc operator()(vc v0, vc v1, vc v2, vc v3) const ;
	//vc operator()(void *p) const ;
#endif

	int func_eq(const vc& v) const ;
	int is_atomic() const;
	hashValueType hashValue() const ;
    //vc translate(VcIO) const;
    virtual vc emitlink(VCArglist *, vc ourname, VcIO) const;

};

#endif
