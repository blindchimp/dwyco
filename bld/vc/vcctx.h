
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
class vc_object;

struct try_handler {
	vc pattern;
	vc catch_expr;
};

class vcctx
{
private:
	DwVecP<functx> maps;
	int ctx;
    functx *cur_ctx;

	int exc_in_progress;
	int break_one;

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

    int unwind_in_progress() const {
		return ret_in_progress() || break_in_progress() || exc_in_progress || dbg_backout;
	}

	int ret_in_progress() const { return cur_ctx->ret_in_progress(); }
	void set_retval(const vc& v) { cur_ctx->set_retval(v); }
	vc retval() const { return cur_ctx->get_retval(); }
	void clear_ret() { cur_ctx->clear_ret(); }

	int break_in_progress() const { return cur_ctx->break_in_progress(); }
	void set_break_level(int n) { cur_ctx->set_break_level(n); }
	void open_loop() { cur_ctx->open_loop(); }
	void close_loop() { cur_ctx->close_loop(); }

	// exception handling support
	void addbackout(const vc& expr) ;
	int cur_ctx_backouts() { return cur_ctx->num_backouts(); }
	void eval_cur_backouts_since(int n) { cur_ctx->eval_backouts_since(n); }
	void set_exc_in_progress() { exc_in_progress = 1; }
	void clear_exc_in_progress() { exc_in_progress = 0; }
	int exc_is_in_progress() const { return exc_in_progress; }

	void set_break_one() { break_one = 1; }
	int check_break_one() { int r = break_one; break_one = 0; return r; }

	// try handler stack
	DwVec<try_handler> handler_stack;
	void push_handler(const vc& pat, const vc& catch_expr) {
		handler_stack.append(try_handler{pat, catch_expr});
	}
	void pop_handler() {
		if(handler_stack.num_elems() > 0)
			handler_stack.set_size(handler_stack.num_elems() - 1);
	}

	// default handler (excdhandle)
	int has_dhandler;
	vc dhandler_pat;
	vc dhandler_fun;

	//
	// debugging
	//
	void dump(VcIO) const;
	int dbg_backout_in_progress() ;
	void set_dbg_backout() ;
	void clear_dbg_backout() ;
};

#endif
