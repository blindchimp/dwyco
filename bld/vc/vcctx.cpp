
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vcfunctx.h"
#include "vcexcctx.h"
#include "vcctx.h"
#include "vcmap.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcctx.cpp 1.46 1997/10/05 17:27:06 dwight Stable $";

vcctx::vcctx() {
	ctx = 0;
	exc_backout = 0;
	exc_doing_backouts = 0;
	backout_handler = 0;
	backout_ctx = 0;
	dbg_backout = 0;
	maps[0] = new functx(313);
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
		maps[ctx]->disable_obj_ctx();
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
}

void
vcctx::close_ctx()
{
	if(ctx <= 0)
		oopanic("ctx underflow");
	// call backout functions if we're backing out
	if(exc_backout)
	{
		// now we start calling backout functions for the
		// context being closed.
		call_backouts_back_to(backout_handler);
    }
	delete maps[ctx];
	--ctx;
#ifdef LHOBJ
	// re-enable the previous object context in case it was
	// disabled on entry.
	maps[ctx]->enable_obj_ctx();
#endif
}

#ifdef LHOBJ
void
vcctx::open_obj_ctx(vc_object *o)
{
	maps[ctx]->open_obj_ctx(o);
}

void
vcctx::close_obj_ctx()
{
	maps[ctx]->close_obj_ctx();
}

void
vcctx::obj_bind(const vc& mem, const vc& val)
{
	maps[ctx]->obj_bind(mem, val);
}

int
vcctx::obj_contains(const vc& mem)
{
	return maps[ctx]->obj_contains(mem);
}

void
vcctx::obj_remove(const vc& mem)
{
	maps[ctx]->obj_del(mem);
}

vc
vcctx::obj_find(const vc& mem)
{
	return maps[ctx]->obj_find(mem);
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
	maps[ctx]->set_base_init(s);
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
	maps[ctx]->add(key, value);
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
force_termination(const char *reason, const vc& excstr)
{
	VcError << reason << "(" << (const char *)excstr << ")\n";
	oopanic("forced program termination.");
}

#ifdef VCDBG
#include "vcdbg.h"
class VcExcDebugNode : public VcDebugNode
{
public:
	vc status;
	vc exc_str;
	
    virtual void printOn(VcIO) ;
    virtual void printOnBrief(VcIO);
    virtual int has_brief();
};

void
VcExcDebugNode::printOn(VcIO o)
{
	printOnBrief(o);
}

void
VcExcDebugNode::printOnBrief(VcIO o)
{
	status.print_top(o);
	o << ": ";
	exc_str.print_top(o);
	o << "\n";
}

int
VcExcDebugNode::has_brief()
{
	return 1;
}


#endif
void
vcctx::excraise(const vc& str, VCArglist *al)
{
	char buf[128];
#ifdef VCDBG
	VcExcDebugNode dbg;
	dbg.exc_str = str;
	dbg.status = "exception raised";
#endif

	if(exc_backout)
	{
		char foo[1024];
		sprintf(foo, "raised %s: raise while backing out?", (const char *)str);
		oopanic(foo);
	}
	if(exc_doing_backouts)
	{
		force_termination("Can't raise exception in backout function", str);
		/*NOTREACHED*/
	 }
	char level = ((const char *)str)[0];
	for(int i = ctx; i >= 0; --i)
	{
		excctx *e;
		excfun *handler;
		if((e = maps[i]->exchandlerfind(str, handler)) != 0)
		{
			// found handler; disable and call backout function
			// associated with it. Note: this might go
			// recursive if another exception is raised in the
			// handler function, so don't assume anything about
			// the state of "this", ie. ctx might change...
			//
			handler->disable();
			vc ret = (*handler)(al);
			if(exc_backout)
			{
				// handler raised exception that wants to backout.
				// we leave the handler disabled in this case
				// since we know that it is not the one that is
				// eventually going to be backed out to. it is the
				// handler found when another exception has re-raised
				// that is going to be used.
                //
				return;
				}
			handler->enable();
			if(level == 'A' && ret != vc("backout"))
            {
            	sprintf(buf, "Function cannot resume after exception %s", (const char *)str);
            	USER_BOMB2(buf);
            }
			// handled ok, now start the backout process
            // if requested.
			// note: we might be able to do it all here,
			// but that might leave other parts of the system
			// confused. Instead, simply set a flag that tells
			// other contexts that we're backing out, and that
            // functions should be called accordingly.
			//
			if(ret == vc("backout"))
			{
				backout_ctx = e;
                backout_handler = handler;
				exc_backout = 1;
				return;
			}
			// handler wants to resume

            return;
		}

	}
	// no handler found, perform default system action
	if(level == 'W')
		return;
	sprintf(buf, "No handler found for %s", (const char *)str);
	USER_BOMB2(buf);
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
	maps[ctx]->add(k, val);
}

int
vcctx::local_contains(const vc& v) {
	return maps[ctx]->contains(v);
}

void
vcctx::local_remove(const vc& k) {
	maps[ctx]->del(k);
}

vc
vcctx::local_find(const vc& k) {
	return maps[ctx]->get(k);
}


// exception handling support
int
vcctx::backed_out_to(excfun *handler) {
	if(!exc_backout) return 0;
	return handler == backout_handler;
	//return maps[ctx]->backed_out_to(handler);
}

excfun *
vcctx::addhandler(const vc& pat, const vc& fun) {
	return maps[ctx]->addhandler(pat, fun);
}

excfun *
vcctx::add_instant_backout_handler(const vc& pat) {
	return maps[ctx]->add_instant_backout_handler(pat);
}

void
vcctx::add_default_handler(const vc& pat, const vc& fun) {
	maps[0]->add_default_handler(pat, fun);
	// maybe someday add local default handlers
}

void
vcctx::drophandler(excfun *handler) {
	maps[ctx]->drop(handler);
}

void
vcctx::addbackout(const vc& expr) {
	maps[ctx]->addbackout(expr);
}

void
vcctx::call_backouts_back_to(excfun *handler) {
	// we reset exc_backout so that the normal call
	// processing flow is re-enabled while in the backout
	// functions. note that raises are not allowed while
	// processing a backout function (program termination
	// results if this is attempted.
	if(!exc_backout)
		oopanic("backing out while not in backout-mode?");
	exc_backout = 0;
	exc_doing_backouts = 1;
	maps[ctx]->call_backouts_back_to(handler);
	exc_doing_backouts = 0;
	exc_backout = 1;
}

void
vcctx::backout_done() { exc_backout = 0; }
int
vcctx::backout_in_progress() {return exc_backout;}
void
vcctx::set_handler_ret(const vc& v) { handler_ret = v; }
vc
vcctx::get_handler_ret() {return handler_ret; }

//
// debugging
//

int
vcctx::dbg_backout_in_progress() {return dbg_backout;}
void
vcctx::set_dbg_backout() {dbg_backout = 1;}
void
vcctx::clear_dbg_backout() {dbg_backout = 0;}
