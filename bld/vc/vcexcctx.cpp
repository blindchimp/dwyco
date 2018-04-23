
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vcexcctx.h"
#include "vcmap.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcexcctx.cpp 1.45 1996/11/17 05:58:35 dwight Stable $";

void force_termination(const char *reason, const vc& excstr);

vc
default_handlerfun::operator()(VCArglist *al)
{
	vc ret = hfun(al);
	if(ret == vc("backout"))
	{
		force_termination("Default handlers cannot backout", (*al)[0]);
		/*NOTREACHED*/
	}
	return ret;
}

int
default_handlerfun::matches(const vc& v)
{
	int pmatch(const char *, const char *);
	if(disabled)
    	return 0;
	const char *pat = excre;
    const char *exc = v;
	return pmatch(pat, exc);
}

vc
handlerfun::operator()(VCArglist *al)
{
	vc ret = hfun(al);
	return ret;
}

vc
instant_backout::operator()(VCArglist *al)
{
	vc ret("backout");
	vc hr(VC_VECTOR);
	for(int i = 0; i < al->num_elems(); ++i)
		hr.append((*al)[i]);
	Vcmap->set_handler_ret(hr);
	return ret;
}

excctx::excctx(functx *backptr)
{
	fctx = backptr;
	ndefault_handlers = 0;
}

excctx::~excctx()
{
	int n = handlers.num_elems();
	for(int i = 0; i < n; ++i)
		delete handlers[i];
}

int
excctx::backed_out_to(excfun *handler)
{
	int i = handlers.num_elems();
	if(i == 0)
		oopanic("no handlers?");
	return handlers[i - 1] == handler;
}


int
excctx::find(const vc& estr, excfun*& handler_out)
{
	int i = handlers.num_elems() - 1;
	for(;i >= 0; --i)
	{
		if(handlers[i]->matches(estr))
		{
			handler_out = handlers[i];
			return 1;
		}
	}
	return 0;
}

void
excctx::unwindto(excfun *handler)
{
	int nh = handlers.num_elems();
	for(--nh;nh >= 0; --nh)
	{
		if(handler == handlers[nh])
			break;
		handlers[nh]->dobackout();
        delete handlers[nh];
	}
	handlers.set_size(nh + 1);
}

void
excctx::drop(excfun *handler)
{
	int nh = handlers.num_elems();
	for(--nh;nh >= 0 && handler != handlers[nh]; --nh)
		delete handlers[nh];
	if(nh >= 0) // toast requested handler as well.
	{
		delete handlers[nh];
		--nh;
    }
	handlers.set_size(nh + 1);
}
	
   	


void
excctx::add(const vc& fun)
{
	handlers.append(new backoutfun(fun));
}

excfun *
excctx::add(const vc& pat, const vc& fun)
{
	excfun *h = new handlerfun(pat, fun);
	handlers.append(h);
	return h;
}

excfun *
excctx::add_instant_backout(const vc& pat)
{
	excfun *h = new instant_backout(pat);
	handlers.append(h);
	return h;
}

void
excctx::add_default(const vc& pat, const vc& fun)
{
	excfun *h = new default_handlerfun(pat, fun);
	handlers.insert(h, ndefault_handlers);
	++ndefault_handlers;
}
