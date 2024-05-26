
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef LHOBJ
#ifndef VCOBJ_H
#define VCOBJ_H
// $Header: g:/dwight/repo/vc/rcs/vcobj.h 1.45 1997/10/05 17:27:06 dwight Stable $

#include "vccomp.h"
#include "vcfac.h"

class vc_object : public vc_composite
{
#ifdef VCDBG
friend class VcObjectDbgNode;
#endif
friend class vc_factory_def;
public:
	//vc_object(vc_factory_def *fac, VC_FACTORY_MAP *mems, const vc& base,
	//	const VC_FORWARD_LIST& forw, const VC_FORWARD_LIST& dele);
    vc_object(const vc& fac_name, VC_FACTORY_MAP *mems);
	~vc_object();

	vc_default *do_copy() const {oopanic("copy object"); return 0;}
	enum vc_type type() const {return VC_OBJECT;}
	vc_object *get_obj() const;
	int is_quoted() const {return TRUE;}
	int find(const vc& key, vc& out);
	int member_select(const vc& member, vc& out, int toplev, vc_object *topobj);
	void printOn(VcIO os) ;
	int obj_bind(const vc& mem, const vc& v, int top = 1);
	
	int mutant;

private:
    const vc& factory_name;
	VC_FACTORY_MAP *members;
	vc base_obj;
	VC_FORWARD_LIST forwards;
	VC_FORWARD_LIST delegates;


};

#endif
#endif
