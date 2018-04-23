
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCEXCCTX_H
#define VCEXCCTX_H
// $Header: g:/dwight/repo/vc/rcs/vcexcctx.h 1.45 1996/11/17 05:58:36 dwight Stable $
#include "vc.h"
#include "dwvecp.h"

//
// VC exception handling is loosely based on the
// exception handler presented in the '85 USENIX proceedings
// by Allman et al. Basically, you can set handlers in
// functions. Handlers specify a regular expression
// that is matched against exceptions that are raised.
// This allows handlers to catch classes of exceptions.
// (Since there is no notion of static type, or type
// in general, there is no other way to do the selection
// easily. Besides, in practice, this works pretty well.)
//
// When an exception is raised, VC searches backwards
// through the call stack for a matching handler.
// If one is found, the handler is disabled, and the
// handler function is called (after arguments are set up.)
// The handler function may: (1) return "resume", in which
// case execution may continue after the raise call; (2) return
// "backout", in which case the call stack is unwound, calling
// backout functions in the process (see below.) (3) raise
// another exception, which starts the whole process over
// again.
//
// The handler function is called in its own context as a
// normal function (so it has access to it's aguments and
// whatever context is available to the caller of "raise")
//
// Backout functions are expressions that are evaled
// whenever an exception backout is in progress. They can
// be used to deallocate resources, for example, when a
// child function unexpectedly raises an exception that backs-out.
// Backout expression are evaluated in the context of the function
// that set them. They are called in the reverse order they
// were set. Backout functions may not raise exceptions.
// [might need to change this to A: exceptions cause
// automatic termination, and everthing else is ignored.]
//
// When an exception has backed-out, the "backed-out" branch
// of the handle construct is executed. The raise call can
// specify a value for the variable "__handler_ret", which
// is installed in the context of the caller of "handle"
// This allows handlers to pass information back to the
// context in which the handler is set.
//
// Some departures:
// Handlers and backout functions can be "cancelled"
// by calling a special function that cancels the previously
// set handler. This is no provision for cancelling an
// arbitrary handler.
//
// Constructs:
// exchandle(pat fun exc_expr normal_expr)
// excbackout(expr)
// exccancel()
// excraise(level excstr args...)
//

class excfun
{
protected:
	vc hfun;	// function to call
	int disabled;

public:

	excfun(const vc& fun) : hfun(fun), disabled(0) {}
	virtual ~excfun() {}
	virtual vc operator()(VCArglist *al) {return hfun(al);}
	void disable() {disabled = 1;}
	void enable() {disabled = 0;}
	int enabled() {return !disabled;}
	virtual int matches(const vc& v) {return 0;}
	// note: with the new "try" thing, we can have
	// multiple handlers set in one context, which
	// means we call backout on non-backouts when we
	// pass thru a try with multiple catches on it.
	virtual void dobackout(){} //{oopanic("backout of non-backout?");}
};

class default_handlerfun : public excfun
{
private: 
	vc excre;		// the exception to handle (string)
	
public:
	default_handlerfun(const vc& re, const vc& fun)
		: excfun(fun), excre(re){}

	virtual vc operator()(VCArglist *al);
    int matches(const vc& v);
};

class handlerfun : public default_handlerfun
{
private:

public:
	handlerfun(const vc& re, const vc& fun)
		: default_handlerfun(re, fun) {}
	~handlerfun() {}
	vc operator()(VCArglist *);
};


class backoutfun : public excfun
{
private:

public:
	backoutfun(const vc& fun) : excfun(fun) {}

	void dobackout() {hfun.force_eval();}

};

// this is used by "try", where the user doesn't
// specify a function to be called on backout.
// it just returns "backout" to cause the exception
// to unwind.
class instant_backout : public handlerfun
{
public:
	instant_backout(const vc& re) : handlerfun(re, vcnil) {}
	vc operator()(VCArglist *);
};

class functx;

class excctx
{
private:
	int ndefault_handlers;
	DwVecP<excfun> handlers;
    functx *fctx;

public:
	excctx(functx *backptr);
    ~excctx();

	void add(const vc& fun);
	excfun *add(const vc& pat, const vc& fun);
	excfun *add_instant_backout(const vc& pat);
	void add_default(const vc& pat, const vc& fun);
	int find(const vc& handler, excfun*& handler_out);
	int backed_out_to(excfun *handler);
    void drop(excfun *handler);
	void unwindto(excfun *);

};

#endif
