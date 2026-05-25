
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
#include "vcfuncal.h"
#include "vcobj.h"
#include "vccvar.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcfunctx.cpp 1.46 1997/10/05 17:27:06 dwight Stable $";

#ifdef PERFHACKS
#include "dwamap.h"
#else
#include "dwmapr.h"
#endif

static long cache_hits;
static long cache_misses;

void
functx::reset_cache_stats()
{
    cache_hits = 0;
    cache_misses = 0;
}

void
functx::print_cache_stats()
{
    long total = cache_hits + cache_misses;
    if(total == 0)
    {
        VcOutput << "vcache: no lookups\n";
        return;
    }
    long pct = (cache_hits * 100) / total;
    VcOutput << "vcache: hits=" << cache_hits
             << " misses=" << cache_misses
             << " pct=" << pct
             << "%\n";
}

functx::functx(int tsize) {
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
    map = new VMAP(tsize);
#endif

#ifdef LHOBJ
	obj_ctx = 0;
	obj_ctx_enabled = 0;
	base_init_in_progress = 0;
#endif
}

functx::~functx()
{
	if(flush_on_close)
		vc_funcall::flush_all_cache();
#ifdef CACHE_LOOKUPS
	vc_cvar::flush_lookup_cache();
#endif
	delete map;
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
functx::add(const vc& key, const vc& value) {
	if(((const char *)key)[0] == '\0')
		USER_BOMB2("can't bind to zero length string");
	if(map->replace(key, value) == 0)
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
functx::find(const vc& key, vc& out) const
{
	hashValueType h = key.hashValue();
	int slot = h & (VAR_CACHE_SIZE - 1);
	if(vc_cache.val[slot] && vc_cache.tag[slot] == h)
	{
		++cache_hits;
		out = *vc_cache.val[slot];
		return 1;
	}
	++cache_misses;
	vc *wp = 0;
	if(map->find(key, out, &wp))
	{
		if(wp)
		{
			vc_cache.tag[slot] = h;
			vc_cache.val[slot] = wp;
		}
		return 1;
	}
#ifdef LHOBJ
	if(obj_ctx != 0 && obj_ctx_enabled)
		return obj_ctx->find(key, out);
#endif
	return 0;
}

int
functx::find2(const vc& key, vc& out, vc*& wp) const
{
	hashValueType h = key.hashValue();
	int slot = h & (VAR_CACHE_SIZE - 1);
	if(vc_cache.val[slot] && vc_cache.tag[slot] == h)
	{
		++cache_hits;
		wp = vc_cache.val[slot];
		out = *wp;
		return 1;
	}
	++cache_misses;
	if(map->find(key, out, &wp))
	{
		if(wp)
		{
			vc_cache.tag[slot] = h;
			vc_cache.val[slot] = wp;
		}
		return 1;
	}
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
	invalidate_cache(v);
#ifdef CACHE_LOOKUPS
	vc_cvar::flush_lookup_cache();
#endif

}

void
functx::invalidate_cache(const vc& key) const
{
	hashValueType h = key.hashValue();
	int slot = h & (VAR_CACHE_SIZE - 1);
	if(vc_cache.tag[slot] == h)
	{
		vc_cache.tag[slot] = 0;
		vc_cache.val[slot] = 0;
	}
}

int
functx::contains(const vc& v) const
{
	return map->contains(v);
}

void
functx::addbackout(const vc& expr) {
    backouts.append(expr);
}

void
functx::eval_backouts()
{
	for(int i = backouts.num_elems() - 1; i >= 0; --i)
	{
		try {
			backouts[i].force_eval();
		} catch(...) {
			VcError << "Exception raised during backout evaluation, terminating.\n";
			oopanic("forced program termination.");
		}
	}
	backouts.set_size(0);
}

void
functx::eval_backouts_since(int n)
{
	for(int i = backouts.num_elems() - 1; i >= n; --i)
	{
		try {
			backouts[i].force_eval();
		} catch(...) {
			VcError << "Exception raised during backout evaluation, terminating.\n";
			oopanic("forced program termination.");
		}
	}
	backouts.set_size(n);
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



