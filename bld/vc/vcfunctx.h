
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
#include "dwvecp.h"

class functx
{
private:
	VMAP *map;
	int flush_on_close;
	DwVec<vc> backouts;
#ifdef LHOBJ
	vc_object *obj_ctx;
	int obj_ctx_enabled;
	int base_init_in_progress;
#endif

	int doing_ret;
	vc retval;
	int break_level;
	int loop_ctrl;

public:
	functx(int tsize = 1);
	~functx();
	void add(const vc& key, const vc& value) ;
	vc get(const vc& key) ;
    int find(const vc& key, vc& out) const ;
    int find2(const vc& key, vc& out, vc*& wp) const ;
	void del(const vc& v) const ;
	int contains(const vc& v) const ;

	void addbackout(const vc& expr) ;
	void eval_backouts() ;
	int num_backouts() { return backouts.num_elems(); }
	void eval_backouts_since(int n);

	int ret_in_progress() const { return doing_ret; }
	void set_retval(const vc& v) { doing_ret = 1; retval = v; }
	vc get_retval() const { return retval; }
	void clear_ret() { doing_ret = 0; retval = vcnil; }

	int break_in_progress() const { return break_level > loop_ctrl; }
	void set_break_level(int n) { break_level -= n; }
	void open_loop() { ++loop_ctrl; }
	void close_loop() { --loop_ctrl; }

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
