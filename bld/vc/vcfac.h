
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef LHOBJ
//
// vc factory definition object
//
// This object can be called in order to create new
// LH objects. If it is selected with a member selection,
// you get meta-info.
//

// $Header: g:/dwight/repo/vc/rcs/vcfac.h 1.45 1997/10/05 17:27:06 dwight Stable $
#ifndef VCFAC_H
#define VCFAC_H

#include "vcfundef.h"
#include "dwvec.h"
#include "dwamap.h"

typedef DwVec<vc> VC_FORWARD_LIST;
typedef DwVecIter<vc> VC_FORWARD_LIST_ITER;
typedef DwAMap<vc, vc> VC_FACTORY_MAP;
typedef DwAMapIter<vc,vc> VC_FACTORY_MAP_ITER;
typedef DwAssocImp<vc,vc> VC_FACTORY_MAP_ASSOC;

class vc_factory_def : public vc_fundef
{
public:
	vc_factory_def();
	vc_factory_def(const vc& name, vc& args, VCArglist *mem_pairs, const vc& base,
		vc& forwards_list, vc& delegates_list);
	~vc_factory_def();
	
	static const vc Basename;
	static const vc Thisname;
	static const vc Postinitname;
	static const vc Preinitname;
	static const vc Postdestroyname;
	static const vc Predestroyname;

private:
        virtual vc do_function_call(VCArglist *, int = 0) ;
        // note: we override here because the simpler
        // function setup is ok for now, we don't create that
        // many objects that it needs all the improvements
        void do_function_initialize(VCArglist *) const;
        void do_function_finalize(VCArglist *)  const;
        virtual void do_arg_setup(VCArglist *) const;

	VC_FACTORY_MAP *member_map;
	vc base_expr;
	VC_FORWARD_LIST forwards;
	VC_FORWARD_LIST delegates;
	long instance_count;
};


#endif
#endif
