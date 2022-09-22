
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCCTX_H
#define VCCTX_H
// $Header: g:/dwight/repo/vc/rcs/vcctx.h 1.46 1997/10/05 17:27:06 dwight Stable $

#include "vc.h"
#include "dwvecp.h"
#include "vcfunctx.h"
//class functx;
class excfun;
class excctx;
class vc_object;

class vcctx
{
private:
	DwVecP<functx> maps;
	int ctx;
	
	// exception handling
	int exc_backout;
    int exc_doing_backouts;
	excfun *backout_handler;
    excctx *backout_ctx;
    vc handler_ret;

    // debugging
    int dbg_backout;
    

public:
	vcctx();
	~vcctx();
	
	int contains(const vc&);
	void add(const vc& k, const vc& val);
	vc get(const vc& key) const;
	vc get2(const vc& key, vc*& wp) const;
    void remove(const vc& key) const;
	int find(const vc& k, vc& out);

	void global_add(const vc& k, const vc& val) ;
	int global_contains(const vc& k) ;
	void global_remove(const vc& v) ;
	vc global_find(const vc& v);

	void local_add(const vc& k, const vc& val) ;
	int local_contains(const vc& v) ;
	void local_remove(const vc& k) ;
	vc local_find(const vc& v);

#ifdef LHOBJ
	void obj_bind(const vc& mem, const vc& val);
	int obj_contains(const vc& mem);
	void obj_remove(const vc& mem);
	vc obj_find(const vc& mem);
	void open_obj_ctx(vc_object *);
	void close_obj_ctx();
	void set_base_init(int);
	int is_base_init();
#endif
	
	void open_ctx(functx * = 0) ;
	void close_ctx();

	void set_retval(const vc& v) { maps[ctx]->set_retval(v); }
    int ret_in_progress() const { return maps[ctx]->ret_in_progress(); }
	vc retval() { return maps[ctx]->get_retval(); }

	void open_loop() {maps[ctx]->open_loop();}
	void close_loop() {maps[ctx]->close_loop();}
    int break_in_progress() const {return maps[ctx]->break_in_progress();}
	void set_break_level(int n) {maps[ctx]->set_break_level(n);}

    int unwind_in_progress() const {
		return ret_in_progress() ||
			break_in_progress() ||
			exc_backout || dbg_backout;
	}

	// exception handling support
	int backed_out_to(excfun *handler) ;

	excfun *addhandler(const vc& pat, const vc& fun);
	excfun *add_instant_backout_handler(const vc& pat);
	void add_default_handler(const vc& pat, const vc& fun) ;
	void drophandler(excfun *handler) ;
	void addbackout(const vc& expr) ;

	void call_backouts_back_to(excfun *handler);
	void backout_done() ;
	int backout_in_progress() ;
	void set_handler_ret(const vc& v) ;
	vc get_handler_ret() ;

	void excraise(const vc& str, VCArglist *al);

	//
	// debugging
	//
	void dump(VcIO) const;
	int dbg_backout_in_progress() ;
	void set_dbg_backout() ;
	void clear_dbg_backout() ;
};

#endif
