
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCFUNCAL_H
#define VCFUNCAL_H
// $Header: g:/dwight/repo/vc/rcs/vcfuncal.h 1.47 1997/10/05 17:27:06 dwight Stable $

#include "vc.h"
#include "vcfunc.h"
#include "vccvar.h"
#include "vcsrc.h"
#include "dwvec.h"
#include "dwlista.h"

//
// class encapsulating a function call
// whose arguments are gathered by the expression
// parsing system in the vc_cvar class.
//
class vc_funcall: public vc_func
{
friend class vc;
friend class vc_cvar;
friend class VcFuncallDbgNode;
friend class functx;

private:
        DwVec<vc> arglist;			// list of argument expressions
        Src_coord_list src_list;
        vc func;					// function expression
	vc_cvar_src_coord start;	// source code start
	vc_cvar_src_coord end;      // source code end

    vc *cached_fun;				// cached value of eval func expr (pointer
								// because no "mutable" yet.)
	unsigned long *cached_when;			// "time" when cached value was computed.

	mutable int is_string;
	
	static unsigned long Cache_counter;	// incremented whenever cache is flushed.
	static void flush_all_cache();
	void flush_cache();

	vc_funcall(const vc& v, const VCList& vl,
                const vc_cvar_src_coord&, const vc_cvar_src_coord&,
                   const Src_coord_list& scl);
	vc_funcall(const vc_funcall& v);
	~vc_funcall();

	void dbg_print(const vc&, int) const;
	void dbg_print(const vc&, VCArglist *) const;
	
	vc eval() const;
    vcy evalarg() const;

	virtual vc_default *do_copy() const {
		vc_funcall *v = new vc_funcall(*this);
		return v;
    }

	virtual void stringrep(VcIO) const;
    virtual vc translate(VcIO) const;

public:
	int func_eq(const vc& v) const {
		return func == ((const vc_funcall &)v).func &&
			arglist == ((const vc_funcall &)v).arglist;
    }

};
#endif
