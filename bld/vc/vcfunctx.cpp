
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcmap.h"
#include "vcfunctx.h"
#include "vcexcctx.h"
#include "vcfuncal.h"
#include "vcobj.h"
#include "vccvar.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfunctx.cpp 1.46 1997/10/05 17:27:06 dwight Stable $";

#ifndef BTYPES
#ifdef PERFHACKS
#include "dwamap.h"
#else
#include "dwmapr.h"
#endif
#else
#include "dwmap.h"
#endif

functx::functx(int tsize) {
	doing_ret = 0;
	loop_ctrl = 0;
	break_level = 0;
	flush_on_close = 0;
#ifdef PERFHACKS
	map = new VMAP(tsize);
	// the map we get here is a fast closed
	// hash with 32 slots. since we are probably
	// caching pointers into this thing, we can't
	// really having it move around. flushing the
	// cache when a physical internal constraint
	// is dealt with which moves the map around
	// would be really confusing (since we would have
	// to recompute or rebind somehow all the bindings and
	// cached bindings in the current function, which
	// might end up binding them to something else unless
	// we were careful.) so, we just say you are limited to
	// 32 items (unless it is the top level) for now.
	// this really needs to be fixed.
	map->no_expand = 1;
#else
	map = new VMAP(vcnil, vcnil, tsize);
#endif
	exc = 0;
#ifdef LHOBJ
	obj_ctx = 0;
	obj_ctx_enabled = 0;
	base_init_in_progress = 0;
#endif
}

functx::~functx()
{
	// if any functions where bound in this context,
	// we have to flush the entire cache since
	// there may be function call objects that got
	// bound to functions created in this context...

	if(flush_on_close)
		vc_funcall::flush_all_cache();
#ifdef CACHE_LOOKUPS
	vc_cvar::flush_lookup_cache();
#endif
	delete map;
    delete exc;
}

void
functx::reset()
{
    if(flush_on_close)
        vc_funcall::flush_all_cache();
#ifdef CACHE_LOOKUPS
    vc_cvar::flush_lookup_cache();
#endif
    delete exc;
    exc = 0;
    doing_ret = 0;
    loop_ctrl = 0;
    break_level = 0;
    flush_on_close = 0;
#ifdef LHOBJ
    obj_ctx = 0;
    obj_ctx_enabled = 0;
    base_init_in_progress = 0;
#endif
}

#ifdef LHOBJ
void
functx::open_obj_ctx(vc_object *o)
{
	if(obj_ctx != 0)
		oopanic("opening another obj ctx");
		
	obj_ctx = o;
	obj_ctx_enabled = 1;
}

void
functx::close_obj_ctx()
{
	if(obj_ctx == 0)
		oopanic("closing closed obj_ctx");
	obj_ctx = 0;
	obj_ctx_enabled = 0;
}

void
functx::enable_obj_ctx()
{
	obj_ctx_enabled = 1;
}

void
functx::disable_obj_ctx()
{
	obj_ctx_enabled = 0;
}

vc
functx::obj_find(const vc& v)
{
	if(!obj_check())
		return vcnil;
	vc out;
	if(obj_ctx->find(v, out))
		return out;
	return vcnil;
}

void
functx::obj_del(const vc& mem)
{
	USER_BOMB2("unimp");
}

int
functx::obj_contains(const vc& mem)
{
	if(!obj_check())
		return 0;
	vc out;
	if(obj_ctx->find(mem, out))
		return 1;
	return 0;
}

void
functx::obj_bind(const vc& mem, const vc& val)
{
	if(!obj_check())
		return;
	obj_ctx->obj_bind(mem, val);
}

int
functx::obj_check()
{
	if(!obj_ctx_enabled || obj_ctx == 0)
	{
		USER_BOMB("object operation must be done inside object context", 0);
	}
	return 1;
}
	
void
functx::set_base_init(int s)
{
	base_init_in_progress = !!s;
}

int
functx::is_base_init()
{
	return base_init_in_progress;
}

#endif


void
functx::add(const vc& key, const vc& value, vc** wp) {
	if(((const char *)key)[0] == '\0')
		USER_BOMB2("can't bind to zero length string");
    if(map->replace(key, value, wp) == 0)
	{
#ifdef CACHE_LOOKUPS
		vc_cvar::flush_lookup_cache();
#endif
	}
	if(value.type() == VC_FUNC)
	{
		vc_funcall::flush_all_cache();
		flush_on_close = 1;
	}
}

vc
functx::get(const vc& key)
{
	vc out;
	if(map->find(key, out))
		return out;
	return vcnil;
}

int
functx::find(const vc& key, vc& out)
{
	if(map->find(key, out))
		return 1;
#ifdef LHOBJ
	if(obj_ctx != 0 && obj_ctx_enabled)
		return obj_ctx->find(key, out);
#endif
	return 0;
}

int
functx::find2(const vc& key, vc& out, vc*& wp)
{
	if(map->find(key, out, &wp))
		return 1;
#ifdef LHOBJ
	if(obj_ctx != 0 && obj_ctx_enabled)
		return obj_ctx->find(key, out);
#endif
	return 0;
}

void
functx::del(const vc& v) const
{
	vc out;
	if(map->find(v, out) && out.type() == VC_FUNC)
		vc_funcall::flush_all_cache();
	map->del(v);
#ifdef CACHE_LOOKUPS
	vc_cvar::flush_lookup_cache();
#endif

}

int
functx::contains(const vc& v) const
{
	return map->contains(v);
}

void
functx::set_break_level(int n) {
	break_level -= n;
	if(break_level < 0)
		USER_BOMB2("too many break levels requested");
}


// support for exception handling

excfun *
functx::addhandler(const vc& pat, const vc& fun) {
	if(exc == 0) exc = new excctx(this);
	return exc->add(pat, fun);
}

excfun *
functx::add_instant_backout_handler(const vc& pat) {
	if(exc == 0) exc = new excctx(this);
	return exc->add_instant_backout(pat);
}

void
functx::add_default_handler(const vc& pat, const vc& fun) {
	if(exc == 0) exc = new excctx(this);
	exc->add_default(pat, fun);
}

void
functx::addbackout(const vc& fun) {
	if(exc == 0) exc = new excctx(this);
	exc->add(fun);
}

// search for handler in this context
excctx *
functx::exchandlerfind(const vc& estr, excfun*& handler_out) {
	if(exc == 0) return 0;
	if(exc->find(estr, handler_out))
		return exc;
	return 0;
}

int
functx::backed_out_to(excfun *handler) {
	if(exc == 0) return 0;
	return exc->backed_out_to(handler);
}
void
functx::drop(excfun *handler) {
	if(exc == 0) return;
	exc->drop(handler);
}

void
functx::dump(VcIO os) const
{
	VMAPIter vmi(map);
	for(; !vmi.eol(); vmi.forward())
	{
       	DwAssocImp<vc,vc> dai = vmi.get();
		dai.get_key().print(os);
		os << " -> " ;
		dai.get_value().print_top(os);
		os << "\n";
	}

}

void
functx::call_backouts_back_to(excfun *handler)
{
	if(exc == 0)
    	return;
	exc->unwindto(handler);
}

