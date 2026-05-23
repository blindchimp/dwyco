
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCFUNCTX_H
#define VCFUNCTX_H
// $Header: g:/dwight/repo/vc/rcs/vcfunctx.h 1.45 1996/11/17 05:58:46 dwight Stable $

#include "vc.h"

#ifdef PERFHACKS
template<class R, class D> class DwAMap;
template<class R, class D> class DwAMapIter;
typedef DwAMap<vc,vc> VMAP;
typedef DwAMapIter<vc,vc> VMAPIter;
#else
template<class R, class D> class DwMapR;
template<class R, class D> class DwMapRIter;
typedef DwMapR<vc,vc> VMAP;
typedef DwMapRIter<vc,vc> VMAPIter;
#endif

class vc_object;

class functx
{
private:
	VMAP *map;
	int flush_on_close;
#ifdef LHOBJ
	vc_object *obj_ctx;
	int obj_ctx_enabled;
	int base_init_in_progress;
#endif

public:
	functx(int tsize = 1);
	~functx();
	void add(const vc& key, const vc& value) ;
	vc get(const vc& key) ;
    int find(const vc& key, vc& out) const ;
    int find2(const vc& key, vc& out, vc*& wp) const ;
	void del(const vc& v) const ;
	int contains(const vc& v) const ;

#ifdef LHOBJ
	// augment function context with object context
	void open_obj_ctx(vc_object *);
	void close_obj_ctx();
	void enable_obj_ctx();
	void disable_obj_ctx();

	void obj_bind(const vc& mem, const vc& val);
	vc obj_find(const vc& mem);
	int obj_contains(const vc& mem);
	void obj_del(const vc& mem);

	void set_base_init(int s);
	int is_base_init();
	
private:
	int obj_check();
public:	
#endif

	//
	// debugging
	//
	void dump(VcIO) const; 	
};

#endif
