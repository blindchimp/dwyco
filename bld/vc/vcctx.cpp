
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vcfunctx.h"
#include "vcctx.h"
#include "vcmap.h"
#include "vcio.h"
#include "vctrt.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcctx.cpp 1.46 1997/10/05 17:27:06 dwight Stable $";

vcctx::vcctx() : pinmap(313)
{
	ctx = 0;
	exc_in_progress = 0;
	break_one = 0;
	dbg_backout = 0;
	has_dhandler = 0;
	maps[0] = new functx(313);
    cur_ctx = maps[0];
}

vcctx::~vcctx()
{
	for(int i = 0; i <= ctx; ++i)
		delete maps[i];
}

void
vcctx::open_ctx(functx *f)
{
	// we don't want references to variables to
	// search back into previous object contexts.
	// so we disable the previous one temporarily
	// while this function context is open.
	// this corresponds to not allowing non-member functions
	// to have access to instance variables.
	// note that if a member function is being set up,
	// the object context will be re-established by the
	// memberfun code.
	
#ifdef LHOBJ
	if(ctx >= 0)
        cur_ctx->disable_obj_ctx();
#endif
	++ctx;
	if(!f)
	{
		maps[ctx] = new functx; // tiny table
	}
	else
	{
		// use the existing function context
		// as a template
	}
    cur_ctx = maps[ctx];
}

void
vcctx::close_ctx()
{
	if(ctx <= 0)
		oopanic("ctx underflow");
	if(exc_in_progress)
		cur_ctx->eval_backouts();
    delete cur_ctx;
	--ctx;
    cur_ctx = maps[ctx];
#ifdef LHOBJ
    cur_ctx->enable_obj_ctx();
#endif
}

#ifdef LHOBJ
void
vcctx::open_obj_ctx(vc_object *o)
{
	cur_ctx->open_obj_ctx(o);
}

void
vcctx::close_obj_ctx()
{
	cur_ctx->close_obj_ctx();
}

void
vcctx::obj_bind(const vc& mem, const vc& val)
{
	cur_ctx->obj_bind(mem, val);
}

int
vcctx::obj_contains(const vc& mem)
{
	return cur_ctx->obj_contains(mem);
}

void
vcctx::obj_remove(const vc& mem)
{
	cur_ctx->obj_del(mem);
}

vc
vcctx::obj_find(const vc& mem)
{
	return cur_ctx->obj_find(mem);
}

int
vcctx::is_base_init()
{
	if(ctx == 0)
		return 0;
	return maps[ctx - 1]->is_base_init();
}

void
vcctx::set_base_init(int s)
{
	cur_ctx->set_base_init(s);
}

#endif


void
vcctx::add(const vc& key, const vc& value)
{
	for(int i = ctx; i >= 0; --i)
	{
		if(maps[i]->contains(key))
        {
			maps[i]->add(key, value);
			return;
		  }
	}
	cur_ctx->add(key, value);
}

vc
vcctx::get(const vc& key) const
{
	vc out;

	for(int i = ctx; i >= 0; --i)
		if(maps[i]->find(key, out))
			return out;
	return vcnil;
}

vc
vcctx::get2(const vc& key, vc*& wp) const
{
	vc out;
	wp = 0;
	for(int i = ctx; i >= 0; --i)
		if(maps[i]->find2(key, out, wp))
			return out;
	return vcnil;
}

int
vcctx::find(const vc& key, vc& out)
{
	for(int i = ctx; i >= 0; --i)
		if(maps[i]->find(key, out))
			return 1;
	return 0;
}
	

int
vcctx::contains(const vc& v)
{
	for(int i = ctx; i >= 0; --i)
		if(maps[i]->contains(v))
			return 1;
	return 0;
}

void
vcctx::remove(const vc& v) const
{
	for(int i = ctx; i >= 0; --i)
		if(maps[i]->contains(v))
		{
			maps[i]->del(v);
			return;
		}
}


void
vcctx::dump(VcIO os) const
{
	for(int i = 0; i <= ctx; ++i)
	{
		os << "\n---\n---Context " << i << "\n---\n";
        maps[i]->dump(os);
	}
}

void
vcctx::global_add(const vc& k, const vc& val) {
	maps[0]->add(k, val);
}

int
vcctx::global_contains(const vc& k) {
	return maps[0]->contains(k);
}

void
vcctx::global_remove(const vc& v) {
	maps[0]->del(v);
}

vc
vcctx::global_find(const vc& k) {
	return maps[0]->get(k);
}

void
vcctx::local_add(const vc& k, const vc& val) {
	cur_ctx->add(k, val);
}

int
vcctx::local_contains(const vc& v) {
	return cur_ctx->contains(v);
}

void
vcctx::local_remove(const vc& k) {
	cur_ctx->del(k);
}

vc
vcctx::local_find(const vc& k) {
	return cur_ctx->get(k);
}


void
vcctx::addbackout(const vc& expr) {
	cur_ctx->addbackout(expr);
}

//
// debugging
//

int
vcctx::dbg_backout_in_progress() {return dbg_backout;}
void
vcctx::set_dbg_backout() {dbg_backout = 1;}
void
vcctx::clear_dbg_backout() {dbg_backout = 0;}
