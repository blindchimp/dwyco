
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vc.cpp 1.49 1998/08/06 04:43:24 dwight Exp $";
#include "vc.h"
#include "vcdeflt.h"
#include "vcdecom.h"
#include "vcstr.h"
#include "vcint.h"
#include "vcdbl.h"
#include "vcnil.h"
#include "vccvar.h"
#include "vcregex.h"
#include "vcsfile.h"
#include "vcwsock.h"
#ifndef DWYCO_NO_UVSOCK
#include "vcuvsock.h"
#endif

vc::vc(double d)
{
	rep = new vc_double(d);
}

vc::vc(int i)
{
	rep = new vc_int((long)i);
}

vc::vc(long i)
{
	rep = new vc_int(i);
}

vc::vc(const char *s)
{
		rep = new vc_string(s);
}

vc::vc(const char *s, int len)
{
        rep = new vc_string(s, len);
}

#ifndef NO_VCEVAL
class VcLexer;
vc::vc(VcLexer &l)
{
	rep = new vc_cvar(&l);
}
#endif
#ifdef VC_DBG_ENVELOPE
#ifdef USE_RCT
#include "rct.h"
#endif
vc::vc()
{
	rep = vc_nil::vcnilrep;
}


vc::vc(const vc& v)
{
	// note: only need this first case if you allow
	// copy constructors for classes derived from vc.
	// if you can avoid this, then you can remove the following
	// if(..)
//	if(v.rep == 0)
//	{
//		rep = 0;
//		return;
//	}
#ifdef USE_RCT
RCQINC(v.rep)
#else
	++v.rep->ref_count;
#endif
	rep = v.rep;
}

vc::~vc()
{
    if(rep == vc_nil::vcnilrep) // base destruct
    return;
// ignore nil destructs, see comment in vcnil.cpp
#ifdef USE_RCT
RCQDEC(rep)
#else
    if(--rep->ref_count == 0)
{
    delete rep;
}
#endif
}

vc&
vc::operator=(const vc& v)
{
	if(&v != this)
	{
#ifdef USE_RCT
RCQINC(v.rep)
RCQDEC(rep)
#else
		++v.rep->ref_count;
		if(rep != vc_nil::vcnilrep && --rep->ref_count == 0) 
		{
			delete rep;
		}
#endif
		rep = v.rep;
	}
	return *this;
}


vc::vc(vc&& v)
{
    rep = v.rep;
    v.rep = vc_nil::vcnilrep;
}


vc&
vc::operator=(vc&& v)
{
    if(this != &v)
    {
    vc_default *tmp = rep;
    rep = v.rep;
    v.rep = tmp;

    }
    return *this;
}
#endif

vc::vc(enum vc_type type, const char *str, long extra_parm)
{
	vc_init(type, str, extra_parm);
}

void
vc::vc_init(enum vc_type type, const char *str, long extra_parm)
{
	switch(type)
	{
	case VC_NIL:
		rep = vc_nil::vcnilrep;
		break;

	case VC_BSTRING:
		rep = new vc_string(str, extra_parm);
		break;
		
	case VC_STRING:
		rep = new vc_string(str);
		break;
	case VC_INT_DTOR:
		rep = new vc_int_dtor(extra_parm, (vc_int_dtor_fun)str);
		break;

	case VC_INT:
		{
		const char *t = str;
		int minus = 1;
		int base = 10;
		if(t[0] == '+')
			++t;
		else if(t[0] == '-')
		{
			++t; minus = -1;
		}
		if(t[0] == '0')
		{
			switch(t[1])
			{
			case 'x':
			case 'X':
				base = 16;
				t += 2;
				break;
			case 't':
			case 'T':
				base = 10;
				t += 2;
				break;
			case 'b':
			case 'B':
				base = 2;
				t += 2;
				break;
			case 'o':
			case 'O':
				base = 8;
				t += 2;
				break;
			default:
				base = 10;
			}
		}
		else
			base = 10;
		char *end;
		long num = strtoul(t, &end, base);
		if(end != str + strlen(str))
			oopanic("bogus long construction");

		rep = new vc_int(num * minus);
		}
		break;

	case VC_DOUBLE:
		rep = new vc_double(atof(str));
		break;
#ifndef NO_VCEVAL
	case VC_CVAR:
		rep = new vc_cvar(str, extra_parm);
		break;
#endif

	case VC_VECTOR:
		rep = new vc_vector(extra_parm);
		break;

	case VC_MAP:
		rep = new vc_map(extra_parm);
		break;

	case VC_LIST:
		rep = new vc_list_set();
		break;

	case VC_SET:
		rep = new vc_set();
		break;

	case VC_SET_N:
		rep = new vc_set(extra_parm);
		break;

	case VC_BAG:
		rep = new vc_bag();
		break;

	case VC_BAG_N:
		rep = new vc_bag(extra_parm);
		break;

	case VC_TREE:
		rep = new vc_tree;
		break;

#ifndef DWYCO_NO_VCREGEX
	case VC_REGEX:
		rep = new vc_regex(str);
        break;
#endif

	case VC_FILE:
		rep = new vc_stdio_file();
		break;

	case VC_SOCKET_STREAM:
		rep = new vc_winsock_stream();
		break;

	case VC_SOCKET_DGRAM:
		rep = new vc_winsock_datagram();
		break;
#ifdef UNIX
	case VC_SOCKET_STREAM_UNIX:
		rep = new vc_winsock_unix();
		break;
#endif
#ifndef DWYCO_NO_UVSOCK
    case VC_UVSOCKET_STREAM:
        rep = new vc_uvsocket;
        break;
#endif

	default:
			oopanic("bad type in vc conversion");
	}
}

//
// use this when you don't know the reference
// count of the vc_default object (you're just
// re-attaching something that already exists
// to another vc) 
// 
void
vc::attach(vc_default *v)
{
#ifdef USE_RCT
RCQINC(v)
RCQDEC(rep)
#else
		if(rep != vc_nil::vcnilrep && --rep->ref_count == 0)
		{
			delete rep;
		}
		++v->ref_count;
#endif
		rep = v;
}

// use this when you know the reference count
// of the vc_default object ahead of time (right
// after is was created, for example.)
//
void
vc::redefine(vc_default *v)
{
#ifdef USE_RCT
RCQDEC(rep)
#else
		if(rep != vc_nil::vcnilrep && --rep->ref_count == 0)
		{
			delete rep;
		}
#endif
		rep = v;
}

int
vc::visited()
{
	return rep->visited();
}

void
vc::set_visited()
{
	rep->set_visited();
}



