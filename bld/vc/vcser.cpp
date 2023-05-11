
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcdeflt.h"
#include "vcint.h"
#include "vcnil.h"
#include "vcdbl.h"
#include "vcstr.h"
#include "vcregex.h"
#include "vcdecom.h"
#include "vcxstrm.h"
#include "vcenco.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcser.cpp 1.49 1998/12/09 05:12:36 dwight Exp $";


long
vc::xfer_out(vcxstream& vcx)
{
	if(visited())
	{
		// generate a chit place holder
		VCXCHIT c = vcx.chit_find(rep);
		char *cp = vcx.out_want(ENCODED_TYPE_LEN);
		if(cp == 0)
			return -1;
		int tlen = encode_type(cp, VC_CHIT);
		char buf[40];
		int clen = encode_long(buf, c);
		if((cp = vcx.out_want(clen)) == 0)
			return -1;
		memcpy(cp, buf, clen);
		return tlen + clen;
	}
	set_visited();
	VCXFERTYPE t = type();
	char *tp = vcx.out_want(ENCODED_TYPE_LEN);
	if(tp == 0)
		return -1;
	int tlen = encode_type(tp, t);
	if(!is_atomic())
		vcx.chit_append(rep);
	long n = rep->xfer_out(vcx);
	if(n < 0)
		return -1;
	return n + tlen;
}

long
vc::xfer_in(vcxstream& vcx)
{
    if(vcx.max_depth == -1)
        return EXIN_PARSE;
    --vcx.max_depth;
    long ret = real_xfer_in(vcx);
    ++vcx.max_depth;
    return ret;
}

long
vc::real_xfer_in(vcxstream& vcx)
{
	VCXFERTYPE t;
	long n = 0;
    int tlen;
    vc_default *nrep;
    {
	char *tp = vcx.in_want(ENCODED_TYPE_LEN);
	if(tp == 0)
		return EXIN_DEV;
	t = decode_type(tp);

	if(t == -1)
		return EXIN_PARSE;
    }
	switch(t)
	{
	case VC_INT:
		nrep = new vc_int;
		break;

	case VC_DOUBLE:
		nrep = new vc_double;
		break;

	case VC_NIL:
		attach(vc_nil::vcnilrep);
        return ENCODED_TYPE_LEN;

	case VC_CHIT:
		{
		char *tp = vcx.in_want(ENCODED_LEN_LEN);
		if(tp == 0)
			return EXIN_DEV;
		tlen = decode_len(tp);
        if(tlen == -1 || tlen == 0)
			return EXIN_PARSE;
		if((tp = vcx.in_want(tlen)) == 0)
			return EXIN_DEV;
        VCXCHIT c = (VCXCHIT)decode_long(tp, tlen);
        if(c == -1)
        	return EXIN_PARSE;
        vc_default *v = vcx.chit_get(c);
        if(v == 0)
            return EXIN_PARSE;
        attach(v);
        return ENCODED_TYPE_LEN + ENCODED_LEN_LEN + tlen;
		}

	case VC_VECTOR:
		nrep = new vc_vector;
		break;

	case VC_MAP:
		nrep = new vc_map; // maybe encode a good table size with map too.
		break;

	case VC_SET:
		// set this to zero here because we know
		// the deserializer will get a count
		// that it can use the size the set so we
		// don't spend a lot of time setting up
		// a large capacity set for nothing
		nrep = new vc_set(0);
		break;

	case VC_BAG:
		nrep = new vc_bag;
		break;

	case VC_TREE:
		nrep = new vc_tree;
		break;

	case VC_LIST:
		nrep = new vc_list_set;
		break;

	case VC_STRING:
		nrep = new vc_string(VcNoinit());
		break;

	// NOTE: this is a problem... it results in
	// some interpreters being able to produce
	// streams that others cannot parse. in addition,
	// it causes problems with security since the
	// regex stuff gets invoked during parsing which
	// i suspect won't pass fuzz tests as readily as
	// the simpler aspects of the encoded stream.
	// maybe think treating regexs in a way that
	// avoids this, like maybe remove them altogether as
	// a "type".
#ifndef DWYCO_NO_VCREGEX
	case VC_REGEX:
		{
		// this is a little bogus...
		// regex's go out exactly as strings, but since
		// there is no way to change a regex after it has
		// been created (and i don't want to change the
		// vc_regex class to use a pointer to Regex)
		// we have to kluge this up a bit.
		// we create a string, read it in normally,
		// then we create a regex from the string
		// object, and then toss the string object.
		nrep = new vc_string(VcNoinit());
		if((n = nrep->xfer_in(vcx)) < 0)
			return n;
		vc_default *nrep2 = new vc_regex((const char *)*nrep);
		delete nrep;
		redefine(nrep2);
		  return n + ENCODED_TYPE_LEN;
		}
#endif

	default:
		return EXIN_PARSE;
	}
	if(!nrep->is_atomic())
		vcx.chit_append(nrep);
	// we redefine here because by putting the
	// new rep into the chit table, it becomes
	// possible for other things to reference
	// it. because of this, we want the reference
	// count on it to be correct (this might not
	// be a big deal, but it is safer anyway...)
	// this also disassociates whatever is referenced
	// by this... which means if something goes wrong,
	// the value is set to nil. otherwise it is
	// set to the xfered value.
	//
	redefine(nrep);
	n = nrep->xfer_in(vcx);
	if(n < 0)
	{
		// if a failure, turn into nil
		// this hopefully will destroy
		// any partial results that
		// may have been created.
		attach(vc_nil::vcnilrep);
    	return n;
	}
	
    return n + ENCODED_TYPE_LEN;
}

