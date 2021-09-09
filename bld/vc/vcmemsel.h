
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCMEMSEL_H
#define VCMEMSEL_H
// $Header: g:/dwight/repo/vc/rcs/vcmemsel.h 1.45 1997/10/05 17:27:06 dwight Stable $

#include "vccomp.h"
#include "vcsrc.h"

//
// object that is used to represent object member selection in the
// syntax tree.
//

class vc_memselect : public vc_composite
{
friend class VcMemSelectDbgNode;
public:
	vc_memselect(const vc& expr, const vc& selector,
                 const vc_cvar_src_coord& s, const vc_cvar_src_coord& e,
                     const vc_cvar_src_coord& selector_coord);
	~vc_memselect();

	vc_default *do_copy() const {oopanic("copy memselect"); return 0;}
	enum vc_type type() const {return VC_MEMSEL;}
	void stringrep(VcIO) const;
	
	vc eval() const;
	void printOn(VcIO);

    vc translate(VcIO o) const;

private:
    vc_cvar_src_coord start;
    vc_cvar_src_coord end;
    vc_cvar_src_coord selector_coord;
    
	vc obj_expr;
	vc selector;
};


#endif
