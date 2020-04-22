
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

class excctx;
class excfun;
class vc_object;

class functx
{
private:
	VMAP *map;
	vc retval;
	int doing_ret;
	int loop_ctrl;
	int break_level;
	int flush_on_close;
	excctx *exc;
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

	void set_break_level(int n) ;

	void set_retval(const vc& v) {retval = v; doing_ret = 1;}
	int ret_in_progress() const {return doing_ret;}
	vc get_retval() {return retval;}

// support looping constructs
	void open_loop() {++loop_ctrl;++break_level;}
	void close_loop() {
		if(loop_ctrl == break_level)
			--break_level;
		if(--loop_ctrl < 0) oopanic("loop underflow");
	}
	int break_in_progress() {
		if(loop_ctrl > break_level)
			return 1;
		return 0;
	}

	// support for exception handling

	excfun *addhandler(const vc& pat, const vc& fun) ;
	excfun *add_instant_backout_handler(const vc& pat) ;
	void add_default_handler(const vc& pat, const vc& fun) ;
	void addbackout(const vc& fun) ;
	

    // search for handler in this context
	excctx *exchandlerfind(const vc& estr, excfun*& handler_out) ;
	void call_backouts_back_to(excfun *);
	int backed_out_to(excfun *handler) ;
	void drop(excfun *handler) ;

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
