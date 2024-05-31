
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// This file contains the glue and initialization
// for the LH language based on the VC class hierarchy.
//
//static char Rcsid[] = "$Header: /e/linux/home/dwight/repo/vc/rcs/vclh.cpp 1.75 1999/03/17 14:57:04 dwight Exp $";
#include <ctype.h>
#include <stddef.h>
#include <time.h>
#if !defined(_Windows) && !defined(_MSC_VER)
#include <unistd.h>
#endif
#include "vc.h"
#include "vcdeflt.h"
#include "vcmap.h"
#include "vcexcctx.h"
#include "vcfundef.h"
#include "vcfile.h"
#include "dwgrows.h"
#include "vclhsys.h"
#include "uricodec.h"
#include "vctrans.h"
#include "vclh.h"
#include "dwdate.h"
#include "zcomp.h"

#ifdef VCDBG
#include "vcdbg.h"
#endif

#include "vcwsock.h"

#include "vclhnet.h"
#ifndef DWYCO_NO_UVSOCK
#include "lhuvsock.h"
#endif


#ifdef LHOBJ
#include "vclhfac.h"
#endif

#include "vcmath.h"

#include "vcxstrm.h"

#ifndef NO_LHCRYPTO
#include "vccrypt2.h"
#include "vcudh.h"
#endif

#include "vclex.h"
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifndef DWYCO_NO_UVSOCK
#include "vcuvsock.h"
#endif

// macos compiler seems fussy about this. seeing as this is a kluge
// we'll just work around it for now.
#ifdef MACOSX
typedef long VC_INT_TYPE;
typedef unsigned long VC_UINT_TYPE;
#else
typedef int64_t VC_INT_TYPE;
typedef uint64_t VC_UINT_TYPE;
#endif

const vc vctrue("t");
const vc vcone(1);
const vc vczero(0);
const vc vcfzero(0.0);
vc vcbitbucket;

VcIOHack VcError(stderr);
VcIOHack VcOutput(stdout);
VcIOHack VcInput(stdin);
void dbg_print_date();


vc
dobacktrace(VCArglist *a)
{
#ifdef VCDBG
	int brief = 1;
	if(a->num_elems() > 0)
	{
		if(!(*a)[0].is_nil())
			brief = 0;
	}
	if(brief)
		VcDbgInfo.backtrace_brief(VcOutput);
	else
		VcDbgInfo.backtrace(VcError);
#else
	if(Vcmap)
		Vcmap->set_dbg_backout();
	else
	{
		// we got here by using VC without LH proper
        DwString m("user panic: ");
        if((*a)[1].type() == VC_STRING)
            m += (const char *)(*a)[1];
        else
            m += "unknown";
		oopanic(m.c_str());
	}
#endif
	return vcnil;
}

void
user_panic(const char *str)
{
	static int did_backtrace;
	dbg_print_date();
	VcError << "runtime error: " << str << "\n";
	if(!did_backtrace)
	{
		VCArglist al;
        al.append(vctrue);
        al.append(str);
		did_backtrace = 1;
		dobacktrace(&al);
	}
}

void
user_warning(const char *str)
{
	dbg_print_date();
	VcError << "runtime warning: " << str << "\n";
#ifdef VCDBG
	VcDbgInfo.backtrace_brief(VcError, 0, 2);
#endif
	
}

// hard to tell where to put this
void
dbg_print_date()
{
	DwDate d;
	DwTime t;
	VcError << d.Month() << "/" << d.DayOfMonth() << "/" << d.Year() << " " <<
		t.AsString().c_str() << " ";
}

#ifdef LHPROF
static
void
dump_histogram(DwHistogram& h, VcIOHack& o)
{
	int n = h.bins.num_elems();
	o << "MAX " << h.max_clipped << " NUM " << h.over_max_samples << "\n";
	for(int i = 0; i < n; ++i)
		o << i << " " << h.bins[i] << "\n";
}
#endif

vc
dump_flat_profile()
{
#ifdef LHPROF
	DwSetIter<vc_func *> i(&vc_func::Func_list);
	// go thru once and compute totals...
	// might be better to use a total from
	// the top level. if a function called "main"
	// exists, we use that as the total, otherwise
	// we use the sum of all the individual totals
	double total_sys = 0;
	double total_time = 0;
	double total_real = 0;
	for(; !i.eol(); i.forward())
	{
		vc_func *f = i.get();
		if(f->call_count == 0)
			continue;
		if(f->name == vc("main"))
		{
			total_sys = f->total_sys_time;
			total_time = f->total_time;
			total_real = f->total_real_time;
			break;
		}
		total_sys += f->total_sys_time;
		total_time += f->total_time;
		total_real += f->total_real_time;
	}
    const char *p = getenv("DWYCO_PROFILE_NAME");
    if(!p)
        p = "prof.out";
    FILE *f = fopen(p, "a");

	if(!f)
    {
        VcIOHackStr err;
        err << "can't open " << p << '\0';
        USER_BOMB(err.ref_str(), vcnil);
    }
    setlinebuf(f);
	VcIOHack o(f);
	o << "--- profile ---\n";
	o << "calls\ttot\tt/callms\t%\ttotsys\tsys/callms\t%sys\treal\trt/callms\t%rt\tname\n";
    for(i.rewind(); !i.eol(); i.forward())
    {
        vc_func *f = i.get();
        if(f->call_count == 0)
            continue;
        char out[1024];
        sprintf(out, "%ld\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%s\n",
                f->call_count,
                f->total_time,
                1000 * f->total_time / f->call_count,
                100 * f->total_time / total_time,

                f->total_sys_time,
                1000 * f->total_sys_time / f->call_count,
                100 * f->total_sys_time / total_sys,

                f->total_real_time,
                1000 * f->total_real_time / f->call_count,
                100 * f->total_real_time / total_real,
                (const char *)f->name);
        o << out;

#ifdef LHPROF_HIST
		o << "USER\n";
		dump_histogram(f->hist_time, o);
		o << "SYS\n";
		dump_histogram(f->hist_sys_time, o);
		o << "REAL\n";
		dump_histogram(f->hist_real_time, o);
#endif
		
	//double total_time;
	//double total_time2;
	//double total_sys_time;
	//double total_sys_time2;
	//double total_real_time;
	//double total_real_time2;
			
	}
#ifdef CRUDE_GC2
extern double	Gc_total_time;
extern double	Gc_total_time2;
extern DwHistogram	Gc_hist_time;
extern double	Gc_total_sys_time;
extern double	Gc_total_sys_time2;
extern DwHistogram	Gc_hist_sys_time;
extern double	Gc_total_real_time;
extern double	Gc_total_real_time2;
extern DwHistogram	Gc_hist_real_time;
extern int Gc_count;
	o << "GC\n";
		o <<  
			Gc_count << "\t" <<
			Gc_total_time << "\t" <<
			1000 * Gc_total_time / Gc_count << "ms\t" <<
			100 * Gc_total_time / total_time << "\t" << 
			Gc_total_sys_time << "\t" <<
			1000 * Gc_total_sys_time / Gc_count << "ms\t" <<
			100 * Gc_total_sys_time / total_sys << "\t" << 
			Gc_total_real_time << "\t" << 
			1000 * Gc_total_real_time / Gc_count << "ms\t" <<
			100 * Gc_total_real_time / total_real << "\t" << 
			"<<gc>>" << "\t" <<
			"\n";
	dump_histogram(Gc_hist_time, o);
	dump_histogram(Gc_hist_sys_time, o);
	dump_histogram(Gc_hist_real_time, o);
#endif
	fclose(f);
#endif
	return vcnil;
}

vc
doputfile(vc& file, vc& item)
{
	vcxstream v(file);
	long len;
	vc_composite::new_dfs();
	if(!v.open(vcxstream::WRITEABLE))
		return vcnil;
	if((len = item.xfer_out(v)) == -1)
		return vcnil;
	if(!v.close(vcxstream::FLUSH))
		return vcnil;
	return vc(len);
}

vc
dogetfile(vc& file, vc& var)
{
    if(var.type() != VC_STRING)
    {
        USER_BOMB("second arg to getfile must be string to bind to", vcnil);
    }
    if(file.eof())
    {
        USER_BOMB("file is at EOF", vcnil);
    }
    vcxstream v(file);
    vc item;
    long len;
    if(!v.open(vcxstream::READABLE))
        return vcnil;
    if((len = item.xfer_in(v)) < 0)
        return vcnil;
    if(!v.close(vcxstream::FLUSH))
        return vcnil;
    Vcmap->local_add(var, item);
    return vc(len);
}

#if 0
vc
dogetfile_test(vc& file, vc& var)
{
	if(var.type() != VC_STRING)
	{
		USER_BOMB("second arg to getfile must be string to bind to", vcnil);
	}
    if(file.eof())
    {
        USER_BOMB("file is at EOF", vcnil);
    }
	vcxstream v(file);
	vc item;
	long len;
    while(1)
    {
        if(!v.open(vcxstream::READABLE, vcxstream::ATOMIC))
            return vcnil;

        if((len = item.xfer_in(v)) < 0)
        {
            if(len == EXIN_DEV && !file.eof())
            {
                v.close(vcxstream::RETRY);
                continue;
            }
            else if(len == EXIN_PARSE)
                return vcnil;
        }
        break;
    }
	if(!v.close(vcxstream::FLUSH))
		return vcnil;
	Vcmap->local_add(var, item);
	return vc(len);
}
#endif


vc
doappend(vc& v1, vc& v2)
{
	v1.append(v2);
	return vcnil;
}

vc
doprepend(vc& v1, vc& v2)
{
	v1.prepend(v2);
	return vcnil;
}

vc
doaddset(vc& v1, vc& v2)
{
	v1.add(v2);
	return vcnil;
}

vc
doremset(vc& v1, vc& v2)
{
	v1.del(v2);
    return vcnil;
}

vc
doremset2(vc& v1, vc& v2)
{
	if(v1.del(v2))
		return vctrue;
    return vcnil;
}

vc
docontains(vc& v1, vc& v2)
{
	if(v1.contains(v2))
		return vctrue;
	return vcnil;
}

vc
doremlast(vc& v1)
{
	return v1.remove_last();
}

vc
doremfirst(vc& v1)
{
	return v1.remove_first();
}

vc
doremfirst2(vc& v1)
{
	vc key;
	vc val = v1.remove_first2(key);
	vc v(VC_VECTOR);
	v[0] = key;
	v[1] = val;
	return v;
}

vc
doempty(vc& v)
{
	if(v.is_empty())
		return vctrue;
	return vcnil;
}

vc
donumelems(vc& v)
{
	vc v2(v.num_elems());
	return v2;
}

vc
dogetindex(vc& v1, vc& v2)
{
	return v1[v2];
}

vc
doputindex(vc& v1, vc& v2, vc& v3)
{
	v1[v2] = v3;
	return v3;
}

vc
doget(vc& set, vc& key)
{
	vc out;

	if(set.find(key, out))
		return out;
	return vcnil;
}



vc
doputkv(vc& map, vc& key, vc& value)
{
	map.add_kv(key, value);
    return vcnil;
}


vc
dolist(VCArglist *a)
{
	vc v(VC_LIST);

	long cnt = a->num_elems();
	for(long i = 0; i < cnt; ++i)
	{
		v.append((*a)[i]);
	}
	return v;
}

vc
doset(VCArglist *a)
{
	vc v(VC_SET);

	long cnt = a->num_elems();
	for(long i = 0; i < cnt; ++i)
	{
		v.add((*a)[i]);
	}
	return v;
}

vc
dobag(VCArglist *a)
{
	vc v(VC_BAG);

	long cnt = a->num_elems();
	for(long i = 0; i < cnt; ++i)
	{
		v.add((*a)[i]);
	}
	return v;
}

vc
doset_size(VCArglist *a)
{
	// first arg is estimate of size
	if(a->num_elems() == 0)
		return(doset(a));
	if((*a)[0].type() != VC_INT)
		USER_BOMB("first are to set2 must be integer size", vcnil);
	
	vc v(VC_SET_N, 0, (long)((*a)[0]));
	long cnt = a->num_elems();
	for(long i = 1; i < cnt; ++i)
	{
		v.add((*a)[i]);
	}
	return v;
}

vc
dobag_size(VCArglist *a)
{
	if(a->num_elems() == 0)
		return(dobag(a));
	if((*a)[0].type() != VC_INT)
		USER_BOMB("first are to bag2 must be integer size", vcnil);
	
	vc v(VC_BAG_N, 0, (long)((*a)[0]));
	long cnt = a->num_elems();
	for(long i = 1; i < cnt; ++i)
	{
		v.add((*a)[i]);
	}
	return v;
}

vc
dostring(VCArglist *a)
{
	char c;
	long cnt = a->num_elems();
	DwGrowingString g(cnt);
	for(long i = 0; i < cnt; ++i)
	{
        const vc& v = (*a)[i];
		switch(v.type())
		{
		case VC_INT:
			if((int)v > 255)
				VcError << "warning: string character >255\n";
			c = (int)v;
			g.append(&c, 1);
			break;
		case VC_STRING:
			g.append((const char *)v, v.len());
			break;
		case VC_NIL:
			break;
		case VC_VECTOR:
			int j;
			int nj;
			nj = v.num_elems();
			for(j = 0; j < nj; ++j)
			{
				if((int)v[j] > 255)
					VcError << "warning: string character >255\n";
				c = (int)v[j];
				g.append(&c, 1);
			}
			break;
		default:
			USER_BOMB("can't create string from type", vcnil);
			break;
		}
	}
	vc ret(VC_BSTRING, g.ref_str(), g.length());
	return ret;
}

vc
dovector(VCArglist *a)
{
	long cnt = a->num_elems();
	vc v(VC_VECTOR, "", cnt);
	for(long i = 0; i < cnt; ++i)
	{
		v[i] = (*a)[i];
	}
	return v;
}

vc
dovectorsize(VCArglist *va)
{
	if(va->num_elems() < 1)
	{
		USER_BOMB("vector constructor must have at least 1 arg", vcnil);
	}
    const vc& v = (*va)[0];
	if(v.type() != VC_INT)
	{
		USER_BOMB("initial vector size must be an integer", vcnil);
	}
	long n = v;
	if(n < 0)
	{
		USER_BOMB("initial vector size must be >= 0", vcnil);
	}
	vc rv(VC_VECTOR, "", n);
	if(va->num_elems() == 2)
	{
		for(int i = 0; i < n; ++i)
			rv[i] = (*va)[1];
	}
	else if(va->num_elems() > 2)
	{
		USER_BOMB("vector constructor cannot have more than 2 args", vcnil);
	}
	return rv;
}

vc
dovector_from_strings(VCArglist *va)
{
	vc ret(VC_VECTOR);
	int n = va->num_elems();
	int out = 0;
	int i;
	int n2;
	const char *c;
	int j;
	
	for(i = 0; i < n; ++i)
	{
		switch((*va)[i].type())
		{
		case VC_INT:
			ret[out++] = (*va)[i];
			break;
		case VC_STRING:
			n2 = (*va)[i].len();
			c = (const char *)(*va)[i];
			for(j = 0; j < n2; ++j)
				ret[out++] = (unsigned char)*c++;
			break;
		case VC_NIL:
			break;
		default:
			USER_BOMB("can't create vector from type", vcnil);
			break;
		}
	}
	return ret;
}

vc
domap(VCArglist *a)
{
	// if first arg is an integer, use that
	// to indicate the size of the mapping.
	// otherwise, assume the default.
	// the rest of the args are should be vectors
	// that are key, value pairs to be initially installed
	// in the mapping.
	long tsize = 31;
	int nargs = a->num_elems();
	if(nargs > 0 && (*a)[0].type() == VC_INT)
	{
		tsize = (*a)[0];
		if(tsize <= 0)
			USER_BOMB("estimated size must be > 0", vcnil);
        a->del(0);
        --nargs;
	}

    vc m(VC_MAP, "", tsize);
	for(int i = 0; i < nargs; ++i)
	{
        vc imap = (*a)[i];
		if(imap.type() != VC_VECTOR || imap.num_elems() != 2)
			USER_BOMB("initial mappings must be 2-element vectors", vcnil);
		m.add_kv(imap[0], imap[1]);
	}
    return m;	 
}

vc
create_tree(VCArglist *a)
{
	// args are should be vectors
	// that are key, value pairs to be initially installed
	// in the tree.
	int nargs = a->num_elems();

    vc m(VC_TREE);
	for(int i = 0; i < nargs; ++i)
	{
        vc imap = (*a)[i];
		if(imap.type() != VC_VECTOR || imap.num_elems() != 2)
			USER_BOMB("initial mappings must be 2-element vectors", vcnil);
		m.add_kv(imap[0], imap[1]);
	}
    return m;	 
}



vc
dostringrep(vc& v)
{
	VcIOHackStr *o = new VcIOHackStr;

	v.stringrep(*o);
	*o << '\0';
    char *s = o->str();
    delete o;
	vc ret(VC_STRING, s);
    delete [] s;
	return ret;
}

#ifndef DWYCO_NO_VCREGEX
static
char *
cpp_strndup(const char *s, int len)
{
	char *ns = new char [len + 1];
	strncpy(ns, s, len);
	ns[len] = '\0';
    return ns;
}

vc
doregex(vc& v)
{
	vc v1(VC_REGEX, v);

	return v1;
}

vc
doresplit(VCArglist *a)
{
	// first argument should be a regex with
	// some \(\) things in it. The second arg
	// should be a string.
	// The re is matched against the string, and
	// the values of the
	// registers are bound to the variables in the
	// rest of the list. If the last variable in the
	// list is a composite, the registers are appended to
	// it.
	int nargs = a->num_elems();
	if(nargs < 2)
		USER_BOMB("split must have at least 2 args", vcnil);

	vc re = (*a)[0];
	if(re.type() != VC_REGEX)
		USER_BOMB("first arg to split must be regex", vcnil);
	int chars_matched = re.re_match((*a)[1]);
	const char *s = (*a)[1].peek_str();
	int len;
	int strt;
	if(chars_matched >= 0)
	{
	if(nargs == 3 && (*a)[2].is_decomposable())
	{
		vc d = (*a)[2];
		
		for(int i = 1; re.re_match_info(strt, len, i); ++i)
		{
			char *ns = cpp_strndup(s + strt, len);
			d.append(vc(VC_STRING, ns));
			delete [] ns;
		}
	}
	else if(nargs >= 3)
	{
		int i;
		for(i = 1; i + 1 < nargs && re.re_match_info(strt, len, i); ++i)
		{
			char *ns = cpp_strndup(s + strt, len);
			(*a)[i + 1].local_bind(vc(VC_STRING, ns));
			delete [] ns;
		}
		if(i + 1 < nargs)
		{
			for(;i + 1 < nargs; ++i)
			{
				(*a)[i + 1].local_bind(vcnil);
			}
		}
	}
    }
	// note: undocumented, re_match can return -2
	// if there is an error of some kind
	if(chars_matched < 0)
		return vcnil;
	if(re.re_match_info(strt, len) == -1)
		oopanic("no regex info?");

	char *ns = cpp_strndup(s + strt, len);

	vc ret(VC_STRING, ns);
	delete [] ns;
    return ret;
}

vc
dorematch(VCArglist *al)
{
	// arg #1 should be a regex
	// arg #2 a string to be matched against
	// opt arg #3 starting pos (default = 0)

	int nargs = al->num_elems();
	if(nargs < 2 || nargs > 3)
		USER_BOMB("match must have 2 or 3 arguments", vcnil);
	int pos = 0;
	vc re = (*al)[0];
	vc str = (*al)[1];
	if(re.type() != VC_REGEX)
		USER_BOMB("first arg to match must be regex", vcnil);
	if(str.type() != VC_STRING)
		USER_BOMB("second arg to match must be string", vcnil);
	const char *strval = str;
    int len = strlen(strval);
	if(nargs == 3)
	{
		if((*al)[2].type() != VC_INT)
        	USER_BOMB("match start pos must be integer", vcnil);
		pos = (*al)[2];
		if(pos < 0 || pos >= len)
			USER_BOMB("pos must be >= 0 and < length of string", vcnil);
	}

	int ret = re.re_match(str, len, pos);

	// note: re_match can return -2 on internal error
	if(ret < 0)
		return vcnil;
	char *matched = cpp_strndup(strval + pos, ret);
	// note: potential problem here, since matching the string "nil"
	// results in nil being returned. ... might have to change the
	// return value for this function to indicate only match/no-match
	// and put value into argument...
	//
	vc matched_ret(VC_STRING, matched);
	delete [] matched;
	return matched_ret;
}

vc
doresearch2(VCArglist *al)
{
	// arg #1 should be a regex
	// arg #2 a string to be searched
	// opt arg #3 starting pos (default = 0)
	// if we were to call directly, but gnustr re class
	// doesn't support this option at the moment.
	// XXXopt arg #4 is range (default = entire string)

	int nargs = al->num_elems();
	if(nargs < 2 || nargs > 3)
		USER_BOMB("search must have 2 to 3 arguments", vcnil);
	int pos = 0;
	vc re = (*al)[0];
	vc str = (*al)[1];
	if(re.type() != VC_REGEX)
		USER_BOMB("first arg to search must be regex", vcnil);
	if(str.type() != VC_STRING)
		USER_BOMB("second arg to search must be string", vcnil);
	int len = str.len();
	if(nargs == 3)
	{
		if((*al)[2].type() != VC_INT)
        	USER_BOMB("search start pos must be integer", vcnil);
		pos = (*al)[2];
		if(pos < 0 || pos >= len)
			USER_BOMB("pos must be >= 0 and < length of string", vcnil);
	}

	int matchlen; 
    int ret = re.re_search(str, len, matchlen, pos);

	if(ret == -1)
		return vcnil;
	vc rv(VC_VECTOR);
	rv.append(ret);
	rv.append(matchlen);
	return rv;
}

vc
doresearch(VCArglist *al)
{
	// arg #1 should be a regex
	// arg #2 a string to be searched
	// opt arg #3 starting pos (default = 0)
	// if we were to call directly, but gnustr re class
	// doesn't support this option at the moment.
	// XXXopt arg #4 is range (default = entire string)

	int nargs = al->num_elems();
	if(nargs < 2 || nargs > 3)
		USER_BOMB("search must have 2 to 3 arguments", vcnil);
	int pos = 0;
	vc re = (*al)[0];
	vc str = (*al)[1];
	if(re.type() != VC_REGEX)
		USER_BOMB("first arg to search must be regex", vcnil);
	if(str.type() != VC_STRING)
		USER_BOMB("second arg to search must be string", vcnil);
	const char *strval = str;
	int len = str.len();
	if(nargs == 3)
	{
		if((*al)[2].type() != VC_INT)
        	USER_BOMB("search start pos must be integer", vcnil);
		pos = (*al)[2];
		if(pos < 0 || pos >= len)
			USER_BOMB("pos must be >= 0 and < length of string", vcnil);
	}

	int matchlen; 
    int ret = re.re_search(str, len, matchlen, pos);

	if(ret == -1)
		return vcnil;
	char *matched = cpp_strndup(strval + ret, matchlen);
	// note: potential problem here, since matching the string "nil"
	// results in nil being returned. ... might have to change the
	// return value for this function to indicate only match/no-match
	// and put value into argument...
	//
	vc matched_ret(VC_STRING, matched);
	delete [] matched;
	return matched_ret;
}


#endif
#ifndef NO_VCEVAL
vc
dofunbuild(const char *name, VCArglist *a, int sty, int decrypt)
{

	// assumes the arglist is a
	// a list of parameter names
	// that are to be entered into the local context of the
	// function, and then bound to the corresponding
	// atoms in the call.
	// The last atom in the arg list is assumed to be the
	// function definition. It is compiled and the result
	// is bound to the original string.
	//
	
	// special check-- if the user has passed in another
	// fundef, they are probably just aliasing something.
	// instead of generating a new fundef object, and
	// relying on eval to get to the right object, just
	// return the fundef object that is already there...
	// this makes the debugging stuff work as expected also...

	// note: assume arg list is already checked...
	
	int n = a->num_elems();
	vc fv = (*a)[n - 1];
	if(fv.type() == VC_FUNC)
		return fv;
	vc_fundef *vf = new vc_fundef(name, a, sty, decrypt);
	vc v;
	v.redefine(vf);
	return v;
}


static
vc
check_fun_arglist(VCArglist *a)
{
	if(a->num_elems() < 2)
		USER_BOMB("function def must have >= 2 args", vcnil);
		
	vc f = (*a)[0];
	if(f.type() != VC_STRING)
		USER_BOMB("can't bind function def to non-string", vcnil);

	a->del(0);
    return f;
}


vc
dofundef(VCArglist *a)
{
	vc f = check_fun_arglist(a);
	if(!f.is_nil())
        f.bind(dofunbuild(f, a, VC_FUNC_NORMAL, 0));
	return vcnil;
}


vc
dogfundef(VCArglist *a)
{
	vc f = check_fun_arglist(a);
	if(!f.is_nil())
        f.global_bind(dofunbuild(f, a, VC_FUNC_NORMAL, 0));
	return vcnil;
}


vc
dolfundef(VCArglist *a)
{
	vc f = check_fun_arglist(a);
	if(!f.is_nil())
        f.local_bind(dofunbuild(f, a, VC_FUNC_NORMAL, 0));
	return vcnil;
}


vc
dospecial_fundef(VCArglist *a)
{
	//
	// first arg to special fun def is a string
	// if character telling how special the function
	// is...
	// g = global
	// l = local
	// v = varadic
	// c = construct
	// d = don't eval args
	// e = if def is a string, decrypt before lexing
	// next args are as in normal function definition.

	typedef void (vc::*pmf)(const vc&) const;
	pmf binder = &vc::bind;

	if(a->num_elems() < 3)
		USER_BOMB("scompile must have >= 3 args", vcnil);

	if((*a)[0].type() != VC_STRING)
		USER_BOMB("first arg to scompile must be style indicator string", vcnil);
		
	const char *ft = (*a)[0];
	int style = VC_FUNC_NORMAL;
	int decrypt = 0;
	while(*ft)
	{
		switch(*ft)
		{
		case 'g':
			binder = &vc::global_bind;
			break;

		case 'l':
			binder = &vc::local_bind;
			break;

		case 'v':
			style |= VC_FUNC_VARADIC;
			break;

		case 'c':
			style |= VC_FUNC_CONSTRUCT;
			break;

		case 'd':
			style |= VC_FUNC_DONT_EVAL_ARGS;
			break;

		case 'e':
			decrypt = 1;
			break;

		default:
			USER_BOMB("bad function spec (must be from [glvcde])", vcnil);
			/*NOTREACHED*/
		}
		++ft;
	}
	a->del(0);
	vc name = check_fun_arglist(a);
	if(!name.is_nil())
		(name.*binder)(dofunbuild(name, a, style, decrypt));
	if((style&(VC_FUNC_CONSTRUCT)) && (style&VC_FUNC_VARADIC))
	{
		VcError << "warning: function ";
		VcError << (const char *)name << " ";
		VcError << "is varadic construct, __lh_varargs bug may bite you.\n";
	}
	return vcnil;
}


vc
dolambda(VCArglist *a)
{
	if(a->num_elems() < 1)
		USER_BOMB("lambda must have at least one argument", vcnil);
    return dofunbuild("lambda-function", a, VC_FUNC_NORMAL, 0);
}

vc
doslambda(VCArglist *a)
{
	if(a->num_elems() < 2)
		USER_BOMB("lambda must have at least two arguments", vcnil);
	//
	// first arg to special fun def is a string
	// if character telling how special the function
	// is...
	// v = varadic
	// c = construct
	// d = don't eval args
	// e = if def is a string, decrypt before lexing
	// next args are as in normal function definition.

	if((*a)[0].type() != VC_STRING)
		USER_BOMB("first arg to slambda must be style indicator string", vcnil);
		
	const char *ft = (*a)[0];
	int style = VC_FUNC_NORMAL;
	int decrypt = 0;
	while(*ft)
	{
		switch(*ft)
		{
		case 'v':
			style |= VC_FUNC_VARADIC;
			break;

		case 'c':
			style |= VC_FUNC_CONSTRUCT;
			break;

		case 'd':
			style |= VC_FUNC_DONT_EVAL_ARGS;
			break;

		case 'e':
			decrypt = 1;
			break;

		default:
			USER_BOMB("bad function spec (must be from [vcde])", vcnil);
			/*NOTREACHED*/
		}
		++ft;
	}
	if((style&(VC_FUNC_CONSTRUCT)) && (style&VC_FUNC_VARADIC))
	{
		VcError << "warning: lambda ";
		VcError << "is varadic construct, __lh_varargs bug may bite you.\n";
	}
	a->del(0);

    return dofunbuild("lambda-function", a, style, decrypt);
}

vc
dofunmeta(vc v)
{
	return v.funmeta();
}
#endif

vc
dotype(vc& v)
{
	switch(v.type())
	{
	case VC_INT:
		return "int";
	case VC_INT_DTOR:
		return "int_dtor";
	case VC_STRING:
	case VC_BSTRING:
		return "string";
	case VC_DOUBLE:
		return "float";
	case VC_NIL:
		// note: this may have been a case where the
		// special case in the vc ctor checking for "nil"
		// caused this problem. if you really want the 
		// string "nil" from a nil type, use "type2" below.
		return vcnil; // note: this actually returns nil, not a string
	case VC_CVAR:
		return "cvar";
	case VC_FUNC:
		return "func";
	case VC_SET:
	case VC_SET_N:
		return "set";
	case VC_MAP:
		return "map";
	case VC_VECTOR:
		return "vector";
	case VC_LIST:
		return "list";
	case VC_BAG:
	case VC_BAG_N:
		return "bag";
	case VC_REGEX:
		return "regex";
	case VC_FILE:
		return "file";
    case VC_SOCKET:
	case VC_SOCKET_STREAM:
	case VC_SOCKET_DGRAM:
	case VC_SOCKET_STREAM_UNIX:
		return "socket";
    case VC_UVSOCKET_STREAM:
    case VC_UVSOCKET_DGRAM:
    case VC_TSOCKET_DGRAM:
    case VC_TSOCKET_STREAM:
        return "uvsocket";
	case VC_CHIT:
		return "chit";
	case VC_OBJECT:
		return "object";
	case VC_MEMSEL:
		return "memsel";
	case VC_MEMFUN:
		return "memfun";
	case VC_HOMOVCHAR:
		return "homochar";
	case VC_HOMOVLONG:
		return "homolong";
	case VC_HOMOVFLOAT:
		return "homofloat";
	case VC_HOMOVDOUBLE:
		return "homodouble";
	case VC_RECORD:
		return "record";
	case VC_MEMBUF:
		return "membuf";
	case VC_FILTER:
		return "filter";
	case VC_TREE:
		return "tree";
	}
	return "<<unknown>>";
}

// same as above, but always returns a string,
// even for nil objects.
vc
dotype2(vc& v)
{
	v = dotype(v);
	if(v.is_nil())
		return "nil";
	else
		return v;
}

// this is a more detailed type function, useful for
// wrapping other libs and stuff
vc
dotype3(vc& v)
{
	switch(v.type())
	{
	case VC_INT:
		return "int";
	case VC_INT_DTOR:
		return "int_dtor";
	case VC_STRING:
	case VC_BSTRING:
		return "string";
	case VC_DOUBLE:
		return "float";
	case VC_NIL:
		// note: this may have been a case where the
		// special case in the vc ctor checking for "nil"
		// caused this problem. if you really want the 
		// string "nil" from a nil type, use "type2" below.
		return vcnil; // note: this actually returns nil, not a string
	case VC_CVAR:
		return "cvar";
	case VC_FUNC:
		return "func";
	case VC_SET:
	case VC_SET_N:
		return "set";
	case VC_MAP:
		return "map";
	case VC_VECTOR:
		return "vector";
	case VC_LIST:
		return "list";
	case VC_BAG:
	case VC_BAG_N:
		return "bag";
	case VC_REGEX:
		return "regex";
	case VC_FILE:
		return "file";
    case VC_SOCKET:
		return "socket";
	case VC_SOCKET_STREAM:
		return "socket_stream";
	case VC_SOCKET_DGRAM:
		return "socket_dgram";
	case VC_SOCKET_STREAM_UNIX:
		return "socket_stream_unix";
    case VC_UVSOCKET_STREAM:
        return "uvsocket_stream";
    case VC_UVSOCKET_DGRAM:
        return "uvsocket_dgram";
    case VC_TSOCKET_DGRAM:
        return "tsocket_dgram";
    case VC_TSOCKET_STREAM:
        return "tsocket_stream";
	case VC_CHIT:
		return "chit";
	case VC_OBJECT:
		return "object";
	case VC_MEMSEL:
		return "memsel";
	case VC_MEMFUN:
		return "memfun";
	case VC_HOMOVCHAR:
		return "homochar";
	case VC_HOMOVLONG:
		return "homolong";
	case VC_HOMOVFLOAT:
		return "homofloat";
	case VC_HOMOVDOUBLE:
		return "homodouble";
	case VC_RECORD:
		return "record";
	case VC_MEMBUF:
		return "membuf";
	case VC_FILTER:
		return "filter";
	case VC_TREE:
		return "tree";
	}
	return "<<unknown>>";
}


#ifndef NO_VCEVAL
vc
do_exploded_funcall(vc& fun, vc& argvec)
{
	VCArglist a;

	int nargs = argvec.num_elems();
	a.set_size(nargs);
	for(int i = 0; i < nargs; ++i)
        a.append(argvec[i]);
	return fun(&a);
}



vc
doexchandle(VCArglist *a)
{
	// setup up an exception handling context
	// arg #1 is assumed to be the exceptions to be
	// handled as a string that is an sh(1) regular expression.
	//
	// arg #2 is a function to be called in the
	// raiser's context to handle the exception.
	//
	// arg #3 is an expr that is evaluated when an
	// exception is backed-out to this handler. During
	// a backout, this context's retval is assumed to
	// contain return values from the handler that are
	// transferred into this context (to the special
	// variable __handler__ret).
	//
	// arg #4 is the expr to be executed ("try"-ed to use C++ parlance.)
	//
	// special note: exceptions raised during setup of handler
	// context are handled by an enclosing context (if there is one.)
	//
	// implementation notes:
	// There are several cases for dealing with backouts and handlers
	//
	// 1) no backouts or handlers set in function (no change.)
	// 2) backouts but no handlers in function
	//		on normal return, simply toss the backouts (done by
	//		destructors in vcmap)
	//		on backout, call backouts in reverse order (done by
	//		vcmap when function context is closed.)
	//
	// 3) backouts set before single handler
	//		on normal return, toss backouts and handlers (same as 2)
	//		on backout
	//			if handler is not chosen, toss handlers, but call backouts
	//			if handler is chosen, call backouts up to the handler, and
	//				then toss the handler. backouts set before the
	//				the handler are still active.
	//

	vc estr = (*a)[0].eval();
    CHECK_ANY_BO(vcnil);
	if(estr.type() != VC_STRING)
		USER_BOMB("exception identifiers must be strings.", vcnil);
	excfun *handler = Vcmap->addhandler(estr, (*a)[1]);

	vc ret = (*a)[3].force_eval();
	if(Vcmap->backout_in_progress())
		Vcmap->call_backouts_back_to(handler);
	if(Vcmap->backed_out_to(handler))
	{
		// we're it...
		// import the return value from the handler function
		// and then do the exception expression.
		Vcmap->backout_done();
		handler->disable(); // no point in possible recursion here...
		vc("__handler_ret").local_bind(Vcmap->get_handler_ret());
		// note: since the handler return is a global, once
		// we import it, eliminate the global reference so it will
		// be removed properly
		Vcmap->set_handler_ret(vcnil);
		ret = (*a)[2].force_eval();
	}
	// drop the handler and return the value
	Vcmap->drophandler(handler);
	return ret;
}

vc
doexchandle2(VCArglist *a)
{
	// setup up an exception handling context
	// arg #1 is assumed to be the exceptions to be
	// handled as a string that is an sh(1) regular expression.
	//
	// arg #2 is a function to be called in the
	// raiser's context to handle the exception.
	//
	// arg #3 is an expr that is evaluated when an
	// exception is backed-out to this handler. During
	// a backout, this context's retval is assumed to
	// contain return values from the handler that are
	// transferred into this context (to the special
	// variable __handler__ret).
	//
	// arg #4 is the expr to be executed ("try"-ed to use C++ parlance.)
	//
	// special note: exceptions raised during setup of handler
	// context are handled by an enclosing context (if there is one.)
	//
	// implementation notes:
	// There are several cases for dealing with backouts and handlers
	//
	// 1) no backouts or handlers set in function (no change.)
	// 2) backouts but no handlers in function
	//		on normal return, simply toss the backouts (done by
	//		destructors in vcmap)
	//		on backout, call backouts in reverse order (done by
	//		vcmap when function context is closed.)
	//
	// 3) backouts set before single handler
	//		on normal return, toss backouts and handlers (same as 2)
	//		on backout
	//			if handler is not chosen, toss handlers, but call backouts
	//			if handler is chosen, call backouts up to the handler, and
	//				then toss the handler. backouts set before the
	//				the handler are still active.
	//

	vc estr = (*a)[0].eval();
    CHECK_ANY_BO(vcnil);
	if(estr.type() != VC_STRING)
		USER_BOMB("exception identifiers must be strings.", vcnil);
	// note: i changed this because in the past, the
	// expr was just store away for later use, and then
	// when the handler was selected after an excraise, the
	// expr would be evaluated in the context of the *raise*,
	// which is really confusing. now, the function that will be used
	// is the one that is calculated at the time exchandle is called.
	// not sure what i was thinking
	// before...
	vc hfun = (*a)[1].force_eval();
	CHECK_ANY_BO(vcnil);
	if(hfun.type() != VC_FUNC)
	{
		USER_BOMB("arg 2 (handler func) must evaluate to a function.", vcnil);
	}
	// previously, this was only evaled at the time
	// the exception was selected and backed-out to...
	// this can lead to problems where things may not
	// exist at the time the raise call is made. so
	// evaluate it once when the handler is set up in order
	// to allow the user to set things up with existing
	// bindings, then eval it again when we are backed-out to.
	vc excexpr = (*a)[2].eval();
	CHECK_ANY_BO(vcnil);
	excfun *handler = Vcmap->addhandler(estr, hfun);

	vc ret = (*a)[3].force_eval();
	if(Vcmap->backout_in_progress())
		Vcmap->call_backouts_back_to(handler);
	if(Vcmap->backed_out_to(handler))
	{
		// we're it...
		// import the return value from the handler function
		// and then do the exception expression.
		Vcmap->backout_done();
		handler->disable(); // no point in possible recursion here...
		vc("__handler_ret").local_bind(Vcmap->get_handler_ret());
		// note: since the handler return is a global, once
		// we import it, eliminate the global reference so it will
		// be removed properly
		Vcmap->set_handler_ret(vcnil);
		ret = excexpr.force_eval();
	}
	// drop the handler and return the value
	Vcmap->drophandler(handler);
	return ret;
}

vc
dotry(VCArglist *a)
{
	// a simplified version of
	// the exchandle call.
	// arg1 is the expr to be "try"-ed
	// arg2 is exception string to match against, if there is no
	// 	arg2, it is assumed to be "*"
	// arg3 is the "catch" expr, that is evaluated when
	// the exception is backed out.
	// there is no "handler", it is always "backout".
	// you can have multiple handlers, but in that case,
	// all 3 args must be specified
	// arg4 is the next exception string
	// arg5 is the "catch"
	// all the catch exprs are evaled first *then* the
	// handlers are installed (this avoids cases where
	// the eval of a catch expr causes an exception which
	// would be handled in the same try)
	//

	// special cases:
	// no args, just return nil
	if(a->num_elems()  == 0)
		return vcnil;
	// one arg means assume * for handler, and empty catch function
	// XXX FIX ME
	if(a->num_elems() == 1)
	{
		return (*a)[0].force_eval();
	}
	int catch_expr = -1;
	DwVec<vc> excexprs;
	DwVec<vc> estrs;
	DwVecP<excfun> handlers;
	if(a->num_elems() == 2)
	{
		estrs[0] = "*";
		catch_expr = 1;
	}
	else if(a->num_elems() >= 3)
	{
		estrs[0] = (*a)[1].eval();
		CHECK_ANY_BO(vcnil);
		if(estrs[0].type() != VC_STRING)
			USER_BOMB("exception patterns must be strings.", vcnil);
		catch_expr = 2;
	}
	excexprs[0] = (*a)[catch_expr].eval();
	CHECK_ANY_BO(vcnil);
	
	int k = a->num_elems() - catch_expr - 1;
	if(k & 1)
	{
		USER_BOMB("try must have 0, 1, 2, or 3 args, or if more than 3 args, the number in excess of 4 must be even.", vcnil);
	}
	int hi = 1;
	int i;
	for(i = 0; i < k; i += 2, ++hi)
	{
		estrs[hi] = (*a)[i + 3].eval();
		CHECK_ANY_BO(vcnil);
		if(estrs[hi].type() != VC_STRING)
			USER_BOMB("exception patterns must be strings.", vcnil);
		excexprs[hi] = (*a)[i + 4].eval();
		CHECK_ANY_BO(vcnil);
	}


	// previously, this was only evaled at the time
	// the exception was selected and backed-out to...
	// this can lead to problems where things may not
	// exist at the time the raise call is made. so
	// evaluate it once when the handler is set up in order
	// to allow the user to set things up with existing
	// bindings, then eval it again when we are backed-out to.
	int numhandlers = estrs.num_elems();
	for(i = 0; i < numhandlers; ++i)
	{
		excfun *handler = Vcmap->add_instant_backout_handler(estrs[i]);
		handlers[i] = handler;
	}

	vc ret = (*a)[0].force_eval();
	if(Vcmap->backout_in_progress())
	{
		for(i = numhandlers - 1; i >= 0; --i)
		{
			Vcmap->call_backouts_back_to(handlers[i]);
			if(Vcmap->backed_out_to(handlers[i]))
			{
				// we're it...
				// import the return value from the handler function
				// and then do the exception expression.
				Vcmap->backout_done();
				handlers[i]->disable(); // no point in possible recursion here...
				vc("__handler_ret").local_bind(Vcmap->get_handler_ret());
				// note: since the handler return is a global, once
				// we import it, eliminate the global reference so it will
				// be removed properly
				Vcmap->set_handler_ret(vcnil);
				ret = excexprs[i].force_eval();
				break;
			}
		}
	}
	// drop the handlers and return the value
	// note: only have to drop the first handler
	// since this cleans up everything below it too
	Vcmap->drophandler(handlers[0]);
	return ret;
}

vc
doexcdhandle(vc& estr, vc& hfun)
{
// see note above regarding this change. also note
// that we might be able to just remove the "DONT_EVAL"
// attr set below and let the eval happen before the call.
	estr = estr.eval();
    CHECK_ANY_BO(vcnil);
	hfun = hfun.eval();
	CHECK_ANY_BO(vcnil);
	if(hfun.type() != VC_FUNC)
	{
		USER_BOMB("arg 2 (handler func) must evaluate to a function.", vcnil);
	}
	Vcmap->add_default_handler(estr, hfun);
    return vcnil;
}

vc
doexcraise(VCArglist *a)
{
	//
	// raise an exception
	//
	// arg #1 is the exception identifier
	// arg #2-n are arguments to the handler, if
	// one can be found.

	vc estr = (*a)[0];
	if(estr.type() != VC_STRING)
		USER_BOMB("exception identifiers must be strings.", vcnil);
	Vcmap->excraise(estr, a);
	return vcnil;
}

vc
doexcbackout(vc& expr)
{
	Vcmap->addbackout(expr);
	return vcnil;
}

vc
dosethandlerret(vc& expr)
{
	Vcmap->set_handler_ret(expr);
	return vcnil;
}

vc
doif(VCArglist *a)
{
	// assumes the arglist is 2 or 3 items:
	// #1 is condition
	// if #1 evals to non-nil, then #2 is evaled.
	// otherwise, if #3 exists, it is evaled.
#ifdef VCDBG
    auto c = VcDbgInfo.get();
    c->cur_idx = 0;
#endif
    const vc& cond = ((*a)[0]).eval();
    CHECK_ANY_BO(vcnil);
    vc ret;
	// note: no need to check_bo after these evals
    // since it is going to return immediately anyway.
	if(!cond.is_nil())
	{
#ifdef VCDBG
        c->cur_idx = 1;
#endif
		ret = ((*a)[1]).eval();
	}
	else if(a->num_elems() == 3)
	{
#ifdef VCDBG
        c->cur_idx = 2;
#endif
		ret = ((*a)[2]).eval();
	}
    return ret;
}

vc
docand(VCArglist *a)
{
	// assumes args are not evaled
	int n = a->num_elems();
	if(n < 2)
		USER_BOMB("conditional and must have at least 2 args", vcnil);
#ifdef VCDBG
    auto c = VcDbgInfo.get();
#endif
	for(int i = 0; i < n; ++i)
	{
#ifdef VCDBG
        c->cur_idx = i;
#endif
        const vc& ret = ((*a)[i]).eval();
		CHECK_ANY_BO(vcnil);
		if(ret.is_nil())
			return vcnil;
	}
	return vctrue;
}

vc
docor(VCArglist *a)
{
	// assumes args are not evaled
	int n = a->num_elems();
	if(n < 2)
		USER_BOMB("conditional or must have at least 2 args", vcnil);
#ifdef VCDBG
    auto c = VcDbgInfo.get();
#endif
	for(int i = 0; i < n; ++i)
	{
#ifdef VCDBG
        c->cur_idx = i;
#endif
		vc ret = ((*a)[i]).eval();
		CHECK_ANY_BO(vcnil);
		if(!ret.is_nil())
			return vctrue;
	}
	return vcnil;
}

vc
doswitch(VCArglist *a)
{
	// arg #1 is evaluated, and then
	// it is 'eq'ed against the evaled
	// even # arguments. the first non-nil
	// found results in the evaluation of the
	// immediately following argument.
	// if none match, the last argument in the list
	// is evaluated and returned.
	int nargs = a->num_elems();
	if(nargs <= 3 || nargs % 2 != 0)
		USER_BOMB("must be an even number >= 4 arguments to 'switch'", vcnil);
#ifdef VCDBG
    auto c = VcDbgInfo.get();
    c->cur_idx = 0;
#endif
	vc val = (*a)[0].eval();
	CHECK_ANY_BO(vcnil);
    int i;
	for(i = 1; i < nargs - 1; i += 2)
	{
#ifdef VCDBG
        c->cur_idx = i;
#endif
		vc val2 = (*a)[i].eval();
		CHECK_ANY_BO(vcnil);
		if(val == val2)
		{
#ifdef VCDBG
        c->cur_idx = i + 1;
#endif
			vc ret = (*a)[i + 1].eval();
			CHECK_ANY_BO(vcnil);
			return ret;
		}
	}
#ifdef VCDBG
        c->cur_idx = i;
#endif
	vc val2 = (*a)[i].eval();
    CHECK_ANY_BO(vcnil);
	return val2;
}

vc
docond(VCArglist *a)
{

	// arguments are pairs of expressions, with
	// the first in each pair being evaluated.
	// if it turns up non-nil, the corresponding second
	// expr is evaluated, and the value is returned.
	// if there are an odd number of args, the last
	// argument is evaled and the value returned if none
	// of the other evaled exprs are non-nil.
	// if the number of args is even, and no expr's eval
    // non-nil, nil is returned.
    int has_default = 0;
	int nargs = a->num_elems();
	if(nargs % 2)
    {
		has_default = 1;
		--nargs;
    }
#ifdef VCDBG
    auto c = VcDbgInfo.get();
#endif

	for(int i = 0; i < nargs; i += 2)
	{
#ifdef VCDBG
        c->cur_idx = i;
#endif
        vc cnd = (*a)[i].eval();
		CHECK_ANY_BO(vcnil);
        if(!cnd.is_nil())
		{
#ifdef VCDBG
        c->cur_idx = i + 1;
#endif
			vc ret = (*a)[i + 1].eval();
			CHECK_ANY_BO(vcnil);
            return ret;
		}
	}
	if(has_default)
	{
#ifdef VCDBG
        c->cur_idx = nargs;
#endif
		vc ret = (*a)[nargs].eval();
		CHECK_ANY_BO(vcnil);
		return ret;
	}
    return vcnil;
}

vc
doloop(vc& var, vc& lo, vc& hi, vc& expr)
{
#ifdef VCDBG
    auto c = VcDbgInfo.get();
    c->cur_idx = 0;
#endif
    var = var.eval();
    CHECK_ANY_BO(vcnil);
    if(var.type() != VC_STRING)
        USER_BOMB("for loop variable must be string", vcnil);
#ifdef VCDBG
    c->cur_idx = 1;
#endif
	long l = lo.eval();
    CHECK_ANY_BO(vcnil);
#ifdef VCDBG
    c->cur_idx = 2;
#endif
	long h = hi.eval();
    CHECK_ANY_BO(vcnil);

    Vcmap->open_loop();

	long i;
	for(i = l; i <= h; ++i)
	{
		Vcmap->local_add(var, vc(i));
#ifdef VCDBG
    c->cur_idx = 3;
#endif
		expr.eval();
		CHECK_ANY_BO_LOOP;
	}
	Vcmap->close_loop();
	Vcmap->local_add(var, vc(i));
	return vcnil;
}


vc
dowhile(vc& cond, vc& expr)
{
#ifdef VCDBG
    auto c = VcDbgInfo.get();
    c->cur_idx = 0;
#endif
	
	vc a = cond.eval();
    CHECK_ANY_BO(vcnil);
	Vcmap->open_loop();
	while(!a.is_nil())
    {
#ifdef VCDBG
    c->cur_idx = 1;
#endif
		expr.eval();
		if(Vcmap->unwind_in_progress())
			break;
#ifdef VCDBG
    c->cur_idx = 0;
#endif
		a = cond.eval();
		CHECK_ANY_BO_LOOP;
	}
	Vcmap->close_loop();
    return vcnil;
}


vc
doforeach(vc& var, vc& set, vc& expr)
{
#ifdef VCDBG
    auto c = VcDbgInfo.get();
    c->cur_idx = 0;
#endif
	vc b = var.eval();
    CHECK_ANY_BO(vcnil);
	if(b.type() != VC_STRING)
		USER_BOMB("foreach variable must be string", vcnil);
#ifdef VCDBG
    c->cur_idx = 1;
#endif
    vc a = set.eval();
    CHECK_ANY_BO(vcnil);
#ifdef VCDBG
    c->cur_idx = 2;
#endif
	a.foreach(b, expr);
	return vcnil;
}


vc
doreturn(vc& v)
{
	Vcmap->set_retval(v);
	return v;
}


vc
dobreak(vc& v)
{
	if(v.type() != VC_INT)
    	USER_BOMB("break level must be an integer", vcnil);
	long bl = (long)v;
	if(bl < 0)
		USER_BOMB("can't request negative break level", vcnil);
	Vcmap->set_break_level(bl);
	return vcnil;
}

vc
domod(vc& v1, vc& v2)
{
	return v1 % v2;
}

vc
dodiv(vc& v1, vc& v2)
{
	return v1 / v2;
}

vc
domul(vc& v1, vc& v2)
{
	return v1 * v2;
}

vc
doadd(vc& v1, vc& v2)
{
	return v1 + v2;
}

vc
dosub(vc& v1, vc& v2)
{
	return v1 - v2;
}

vc
bindfun(vc& v1, vc& v2)
{
#ifdef PERFHACKS
	if(v1.type() == VC_STRING)
	{
		Vcmap->add(v1, v2);
		return vcnil;
	}
#endif
	v1.bind(v2);
	return vcnil;
}

vc
lbindfun(vc& v1, vc& v2)
{
#ifdef PERFHACKS
	if(v1.type() == VC_STRING)
	{
		Vcmap->local_add(v1, v2);
		return vcnil;
	}
#endif
	v1.local_bind(v2);
	return vcnil;
}

vc
gbindfun(vc& v1, vc& v2)
{
#ifdef PERFHACKS
	if(v1.type() == VC_STRING)
	{
		Vcmap->global_add(v1, v2);
		return vcnil;
	}
#endif
	v1.global_bind(v2);
	return vcnil;
}

#ifdef LHOBJ
vc
obindfun(vc& v1, vc& v2)
{
	Vcmap->obj_bind(v1, v2);
	return vcnil;
}

vc
oboundfun(vc& v)
{
	if(Vcmap->obj_contains(v))
		return vctrue;
	return vcnil;
}

vc
oremovefun(vc& v1)
{
	Vcmap->obj_remove(v1);
	return vcnil;
}

vc
ofindfun(vc& v1)
{
	return Vcmap->obj_find(v1);
}

#endif

vc
boundfun(vc& v)
{
	if(Vcmap->contains(v))
		return vctrue;
	return vcnil;
}

vc
lboundfun(vc& v)
{
	if(Vcmap->local_contains(v))
		return vctrue;
	return vcnil;
}

vc
gboundfun(vc& v)
{
	if(Vcmap->global_contains(v))
		return vctrue;
	return vcnil;
}

// note: this is a little goofy, since there is no
// indication that it actually found anything.
// there is no way to tell (without calling "bound") whether
// the return of nil is "not found" or "found but value is nil"
vc
findfun(vc& v)
{
	vc out;
	if(Vcmap->find(v, out))
		return out;
	return vcnil;
}

vc
lfindfun(vc& v)
{
	return Vcmap->local_find(v);
}

vc
gfindfun(vc& v)
{
	return Vcmap->global_find(v);
}


vc
removefun(vc& v1)
{
	v1.bremove();
	return vcnil;
}

vc
lremovefun(vc& v1)
{
	v1.local_bremove();
	return vcnil;
}

vc
gremovefun(vc& v1)
{
	v1.global_bremove();
	return vcnil;
}

vc
evalfun(vc& v)
{
	return v.force_eval();
}

vc
incrfun(vc& v)
{
	return v + vcone;
}

vc
decrfun(vc& v)
{
	return v - vcone;
}

vc
notfun(vc& v)
{
	if(v.is_nil())
		return vctrue;
	return vcnil;
}


vc
eqfun(vc& v1, vc& v2)
{
	return (v1 == v2) ? vctrue : vcnil;
}

vc
lefun(vc& v1, vc& v2)
{
	return (v1 <= v2) ? vctrue : vcnil;
}

vc
ltfun(vc& v1, vc& v2)
{
	return (v1 < v2) ? vctrue : vcnil;
}

vc
gtfun(vc& v1, vc& v2)
{
	return (v1 > v2) ? vctrue : vcnil;
}

vc
gefun(vc& v1, vc& v2)
{
	return (v1 >= v2) ? vctrue : vcnil;
}

vc
nefun(vc& v1, vc& v2)
{
	return (v1 != v2) ? vctrue : vcnil;
}

vc
orfun(VCArglist *a)
{
	// note: this is old-style, evaluates all
	// args, regardless of return (in fact,
	// that was done before we even got here)
	int n = a->num_elems();
	if(n < 2)
		USER_BOMB("or must have at least 2 args", vcnil);
	for(int i = 0; i < n; ++i)
		if(!(*a)[i].is_nil())
			return vctrue;
	return vcnil;
}

vc
andfun(VCArglist *a)
{
	// note: this is old-style, evaluates all
	// args, regardless of return (in fact,
	// that was done before we even got here)
	int n = a->num_elems();
	if(n < 2)
		USER_BOMB("and must have at least 2 args", vcnil);
	for(int i = 0; i < n; ++i)
		if((*a)[i].is_nil())
			return vcnil;
	return vctrue;
}

vc
xorfun(VCArglist *a)
{
	int n = a->num_elems();
	if(n < 2)
		USER_BOMB("xor must have at least 2 args", vcnil);
	int res = 0;
	for(int i = 0; i < n; ++i)
	{
		res += !(*a)[i].is_nil();
	}
	if(res & 1)
		return vctrue;
	return vcnil;
}

// note: i opted to NOT add "operator&" type things to the base class for
// vc's, (ala add, sub, etc.)
// mainly because it involves a fair amount of work.
// the shift operators don't fit into that framework very well, and it was
// something i had tinkered with a long time ago for allowing direct
// c++ manipulation of vc objects more natural. since these ops don't get
// used that often, it isn't really crucial. if i did do the operators in the vc
// class, it would get rid of the obnoxious VC_INT_TYPE thing...
vc
bnotfun(vc& v)
{
	return vc(~(VC_INT_TYPE)v);
}

vc
bxorfun(vc& v1, vc& v2)
{
	return vc((VC_INT_TYPE)v1 ^ (VC_INT_TYPE)v2);
}

vc
borfun(vc& v1, vc& v2)
{
	return vc((VC_INT_TYPE)v1 | (VC_INT_TYPE)v2);
}

vc
bandfun(vc& v1, vc& v2)
{
	return vc((VC_INT_TYPE)v1 & (VC_INT_TYPE)v2);
}

vc
blsrfun(vc& v1, vc& v2)
{
	return vc((VC_INT_TYPE)(((VC_UINT_TYPE)(VC_INT_TYPE)v1) >> (int)v2));
}
vc
blslfun(vc& v1, vc& v2)
{
	return vc((VC_INT_TYPE)(((VC_UINT_TYPE)(VC_INT_TYPE)v1) << (int)v2));
}

vc
baslfun(vc& v1, vc& v2)
{
	return vc(((VC_INT_TYPE)v1) << (int)v2);
}

vc
basrfun(vc& v1, vc& v2)
{
	return vc(((VC_INT_TYPE)v1) >> (int)v2);
}

vc
dogensym()
{
	static unsigned long i;
	char buf[128];

	++i;
    snprintf(buf, sizeof(buf), "gensym%lu", i);
	return vc(buf);
}

vc
doprint(vc& v)
{
	v.print_top(VcOutput);
	return vcnil;
}

vc
doflush()
{
	VcOutput.flush();
	VcError.flush();
	return vcnil;
}

vc
doflushall()
{
    fflush(NULL);
    return vcnil;
}

vc
docontents_of(vc& file)
{
	if(file.type() != VC_FILE)
		USER_BOMB("arg to contents-of must be file", vcnil);
	DwGrowingString s;
	char buf[1024];
	long len = sizeof(buf);
	while(file.read(buf, len))
	{
        // note: this is only "non-lh" because we use this
        // somewhere in a regular c++ program without LH. duh.
        NONLH_CHECK_ANY_BO(vcnil);
		s.append(buf, len);
		len = sizeof(buf);
	}
	vc v(VC_BSTRING, s.ref_str(), s.length());
	return v;
}

vc
dofprint(vc& file, vc& item)
{
	if(file.type() != VC_FILE)
		USER_BOMB("first arg to fprint must be file", vcnil);
	file << item;
	return vcnil;
}

vc
dofread(vc& file, vc& len)
{
	if(file.type() != VC_FILE)
		USER_BOMB("first arg must be file", vcnil);
	if(len.type() != VC_INT)
		USER_BOMB("second arg must be integer", vcnil);

	long l = (int)len;
	char *buf = new char [l];

	if(!file.read(buf, l))
	{
		delete [] buf;
		CHECK_ANY_BO(vcnil);
		return vcnil;
	}
	vc ret(VC_BSTRING, buf, l);
	delete [] buf;
	return ret;
}

vc
dofgets(vc& file)
{
	if(file.type() != VC_FILE)
		USER_BOMB("first arg must be file", vcnil);

	char buf[8192];

	if(!file.fgets(buf, sizeof(buf)))
	{
		CHECK_ANY_BO(vcnil);
		return vcnil;
	}
	vc ret(buf);
	return ret;
}

vc
dofputs(vc& file, vc& str)
{
	if(file.type() != VC_FILE)
		USER_BOMB("first arg must be file", vcnil);
	if(str.type() != VC_STRING)
		USER_BOMB("second arg must be string", vcnil);
	const char *buf = (const char *)str;
	long len = str.len();
	if(file.write(buf, len) != 1)
	{
		CHECK_ANY_BO(vcnil);
		return vcnil;
	}
	return vc(len);
}

// readline, except it doesn't strip the whitespace from the
// beginning.
vc
doreadline2(vc& file)
{
	if(file.type() != VC_FILE)
		USER_BOMB("first arg must be file", vcnil);

	char buf;
    long len = sizeof(buf);
	long i = 0;
	char *oline = new char [128];
	long curlen = 128;
	int gotone = 0;
	while(file.read(&buf, len))
	{
		if(len == 0)
			break;
		len = 1;
		gotone = 1;
		if(buf == '\n' || buf == '\r')
			break;
		oline[i++] = buf;
		if(i >= curlen)
		{
			char *nline = new char[curlen + 128];
			curlen += 128;
			strncpy(nline, oline, i);
			delete [] oline;
			oline = nline;
		}
	}
	CHECK_ANY_BO(vcnil);
	oline[i] = '\0';
	vc ret;

	if(gotone == 0)
		ret = vcnil;
	else
		ret = vc(oline);

	delete [] oline;
	return ret;
}

vc
doreadline(vc& file)
{
	if(file.type() != VC_FILE)
		USER_BOMB("first arg must be file", vcnil);

	char buf;
    long len = sizeof(buf);
	long i = 0;
	char *oline = new char [128];
	long curlen = 128;
    int skipws = 1;
	while(file.read(&buf, len))
	{
		if(len == 0)
			break;
		len = 1;
		if(buf == '\n' || buf == '\r' || (skipws && isspace(buf)))
		{
			if(skipws)
				continue;
			break;
		}
		skipws = 0;
		oline[i++] = buf;
		if(i >= curlen)
		{
			char *nline = new char[curlen + 128];
			curlen += 128;
			strncpy(nline, oline, i);
			delete [] oline;
			oline = nline;
		}
	}
	CHECK_ANY_BO(vcnil);
	oline[i] = '\0';
	vc ret;

	if(i == 0)
		ret = vcnil;
	else
		ret = vc(oline);

	delete [] oline;
	return ret;
}

#ifndef NO_VCEVAL

vc
doreadatoms(vc& file)
{
	vc line = doreadline(file);
	if(line.is_nil())
		return vcnil;

	const char *l = line;
	VcIOHackStr errs;
	VcLexerString lexer((char *)l, errs);
	VcLexer::Token tok;
	VcLexer::Atom atom_type;
	const char *tokval;
	long toklen;

	vc v(VC_VECTOR);
	long i = 0;

	while((tok = lexer.next_token(tokval, toklen, atom_type)) != VcLexer::EOS)
	{
		if(lexer.lexical_error)
		{
			USER_BOMB("lexer had problems with input", vcnil);
			/*NOTREACHED*/
		}
		if(tok != VcLexer::ATOM) // strip out specials
			continue;
		enum vc_type vct = VC_INT;
		switch(atom_type)
		{
		case VcLexer::INTEGER:
			vct = VC_INT;
			break;
		case VcLexer::FLOAT:
			vct = VC_DOUBLE;
			break;
		case VcLexer::STRING:
			vct = VC_STRING;
			// in keeping with our parser, we change strings of
			// "nil" to vcnil
			if(toklen == 3 && tokval[0] == 'n' && tokval[1] == 'i' && tokval[2] == 'l')
				vct = VC_NIL;
			break;
		default:
			oopanic("bad atom_type");
			/*NOTREACHED*/
		}
		v[i++] = vc(vct, tokval);
	}
	return v;
}
#endif

static
int
vc_file_error(vc *vf)
{
	vc_file *v = (vc_file *)vf;
	if(v->emode == EXCEPTIONS)
	{
		VCArglist a;
        a.append(v->errvc);
		vc v2;
		v2.attach(v);
        a.append(v2);
        a.append(v->errvc1);
        a.append(v->errvc2);
		Vcmap->excraise(v->errvc, &a);
		CHECK_ANY_BO(VC_FILE_BACKOUT);
		return VC_FILE_RESUME;
	}
	return VC_FILE_BACKOUT;
}

vc
doopenfile(vc& filename, vc& mode)
{

	vc nfile(VC_FILE);
	nfile.set_err_callback(vc_file_error);

	vcfilemode fm;
	switch(((const char *)mode)[0])
	{
	case 'r':
		fm = VCFILE_READ;
		break;

	case 'w':
		fm = VCFILE_WRITE;
		break;
		
	case 'a':
		fm = VCFILE_APPEND;
		break;

	default:
		USER_BOMB("second arg to openfile must be from [rwa]", vcnil);
	}
	if(strlen(mode) > 1)
	{
		if(((const char *)mode)[1] == 'b')
			fm |= VCFILE_BINARY;
	}

	if(nfile.open(filename, fm))
		return nfile;
	CHECK_ANY_BO(vcnil);
    return vcnil;
}

vc
doclosefile(vc& file)
{
	file.close();
	CHECK_ANY_BO(vcnil);
	return vcnil;
}

vc
doseekfile(vc& file, vc& pos, vc& whence)
{
	long lp = (long) pos;
    int w = SEEK_SET;
	if(whence == vc("set"))
		w = SEEK_SET;
	else if(whence == vc("current"))
		w = SEEK_CUR;
	else if(whence == vc("end"))
		w = SEEK_END;
	else
	{
		USER_BOMB("seek whence must be set, current, or end", vcnil);
	}

	file.seek(lp, w);
	CHECK_ANY_BO(vcnil);
	return vcnil;
}


vc
doexit(vc& v)
{
 	long ecode;
 	if(v.type() != VC_INT)
 	{
 		if(v.is_nil())
 			ecode = 0;
 		else
 			USER_BOMB("exit needs one integer argument", vcnil);
 	}
 	else
 		ecode = v;
 	exit((int)ecode);
	/*NOTREACHED*/
}

vc
docopy(vc& v)
{
	return v.copy();
}

vc
dovoid(VCArglist *)
{
	return vcnil;
}

vc
doprog(VCArglist *)
{
	return vcnil;
}

// this function is useful, but really problematic, since
// we can't really guess how big the result will be.
// do some wild stuff and try twice if we need to get the size
// right.
// also, the type stuff is goofy, you'll have to get your formats
// right in the format string based on what kinda interpreter you
// are using, which is bad. need a non-printf based formatter i think.
vc
vclh_fmt(vc& item, vc& fmt)
{
	char s[4096];
	size_t len;
	switch(item.type())
	{
	case VC_INT:
		len = snprintf(s, sizeof(s), fmt, (VC_INT_TYPE)item);
		if(len >= sizeof(s))
		{
			size_t new_len = len + 1;
			char *a = new char[new_len];
			len = snprintf(a, new_len, fmt, (VC_INT_TYPE)item);
			if(len >= new_len)
			{
				VcError << "warning: wierd snprintf return, output truncated.\n";
                a[new_len - 1] = 0;
			}
			vc ret(VC_BSTRING, a, new_len);
			delete [] a;
			return ret;
		}
		return vc(VC_BSTRING, s, len);
		break;
	case VC_STRING:
	case VC_NIL:
		len = snprintf(s, sizeof(s), fmt, (const char *)item);
		if(len >= sizeof(s))
		{
			size_t new_len = len + 1;
			char *a = new char[new_len];
			len = snprintf(a, new_len, fmt, (const char *)item);
			if(len >= new_len)
			{
				VcError << "warning: wierd snprintf return, output truncated.\n";
                a[new_len - 1] = 0;
			}
			vc ret(VC_BSTRING, a, new_len);
			delete [] a;
			return ret;
		}
		return vc(VC_BSTRING, s, len);
		break;
	case VC_DOUBLE:
		len = snprintf(s, sizeof(s), fmt, (double)item);
		if(len >= sizeof(s))
		{
			size_t new_len = len + 1;
			char *a = new char[new_len];
			len = snprintf(a, new_len, fmt, (double)item);
			if(len >= new_len)
			{
				VcError << "warning: wierd snprintf return, output truncated.\n";
                a[new_len - 1] = 0;
			}
			vc ret(VC_BSTRING, a, new_len);
			delete [] a;
			return ret;
		}
		return vc(VC_BSTRING, s, len);
		break;
	default:
		USER_BOMB("can't format non-atomics", vcnil);
	}
	return vcnil;
}


vc
vclh_atol(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't atol non-string", vcnil);
	
	return vc((long)atol(s));
}

vc
vclh_strlen(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't strlen a non-string", vcnil);
	return vc(s.len());
}

vc
vclh_substr(vc& s, vc& pos, vc& len)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't substr a non-string", vcnil);
	if(pos.type() != VC_INT)
		USER_BOMB("pos must be integer", vcnil);
	if((int)pos < 0 || (int)pos > s.len())
		USER_BOMB("pos out of range", vcnil);
	static vc end("end");
	if(len.type() == VC_STRING && len == end) // go to end of string
	{
		len = (int)s.len() - (int)pos;
	}
	else if(len.type() != VC_INT)
		USER_BOMB("len must be integer or \"end\"", vcnil);
	if((int)len < 0 || (int)pos + (int)len > s.len())
		USER_BOMB("pos + len out of range", vcnil);
	const char *a = (const char *)s;
	vc v(VC_BSTRING, a + (int)pos, (int)len);
	return v;
}

vc
vclh_strspn(vc& s, vc& accept_chars)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't strspn a non-string", vcnil);
	if(accept_chars.type() != VC_STRING)
		USER_BOMB("second arg to strspn must be a string", vcnil);
	const char *ss = (const char *)s;
	const char *acs = (const char *)accept_chars;
	size_t len = strspn(ss, acs);
	return (int)len;
}

vc
vclh_strcspn(vc& s, vc& accept_chars)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't strcspn a non-string", vcnil);
	if(accept_chars.type() != VC_STRING)
		USER_BOMB("second arg to strcspn must be a string", vcnil);
	const char *ss = (const char *)s;
	const char *acs = (const char *)accept_chars;
	size_t len = strcspn(ss, acs);
	return (int)len;
}

vc
vclh_strstr(vc& s, vc& tofind)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't strstr a non-string", vcnil);
	if(tofind.type() != VC_STRING)
		USER_BOMB("second arg to strstr must be a string", vcnil);
	const char *ss = (const char *)s;
	const char *acs = (const char *)tofind;
	
	int l = tofind.len();
	if(l > s.len())
		return vcnil;
	if(l == 0)
		return vcnil;
	int count = s.len();
	for(int i = 0; i < count - (l - 1); ++i)
	{
		if(memcmp(&ss[i], acs, l) == 0)
			return i;
	}
	return vcnil;
}

#ifdef PARSEDATE
extern "C" time_t parsedate(char *, void *);

vc
vclh_parsedate(vc& s)
{
	if(s.type() != VC_STRING)
	{
		USER_BOMB("arg to parsedate must be a string", vcnil);
	}
	const char *str = (const char *)s;
	vc ret(parsedate((char *)str, 0));
	return ret;
}
#endif


vc
vclh_tolower(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't tolower a non-string", vcnil);
    int n = s.len();
    char *a = new char[n];
    const char *b = (const char *)s;
    for(int i = 0; i < n; ++i)
      a[i] = tolower(b[i]);
    vc v(VC_BSTRING, a, n);
    delete [] a;
    return v;
}

#endif
vc
vclh_serialize(vc& v)
{
	vcxstream vcx((char *)0, (long)128 * 1024, vcxstream::CONTINUOUS);
	long len;
	vc_composite::new_dfs();
	if(!vcx.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
		return vcnil;
	if((len = v.xfer_out(vcx)) == -1)
		return vcnil;

	vc ret(VC_BSTRING, vcx.buf, vcx.cur - vcx.buf);
	return ret;
}

vc
vclh_deserialize(vc& v, vc& out)
{
	if(v.type() != VC_STRING)
	{
		USER_BOMB("first arg to deserialize must be a string", vcnil);
	}
	if(out.type() != VC_STRING)
	{
		USER_BOMB("second arg to deserialize must be a string to bind to", vcnil);
	}
	vcxstream vcx((const char *)v, v.len(), vcxstream::FIXED);

	vc item;
	long len;
	if(!vcx.open(vcxstream::READABLE))
	{
		return vcnil;
	}
	if((len = item.xfer_in(vcx)) < 0)
	{
		return vcnil;
	}
	Vcmap->local_add(out, item);
	return vctrue;
}


extern "C" { 
void dump_free();
 };

vc
vclh_to_hex(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't to-hex a non-string", vcnil);
    const char *a = (const char *)s;
    int len = s.len();
    char *d = new char[len * 2 + 1];
    for(int i = 0; i < len; ++i)
    {
        sprintf(&d[2 * i], "%02x", a[i] & 0xff);
    }
    vc r(VC_BSTRING, d, (long)len * 2);
    delete [] d;
    return r;
}

static
int
hexd(char a)
{
    if(a >= '0' && a <= '9')
        return a - '0';
    if(a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    return 0;
}

vc
vclh_from_hex(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't from-hex a non-string", vcnil);
    const char *a = (const char *)s;
    int len = s.len();
    char *d = new char[len / 2];
    for(int i = 0; i < len / 2; ++i)
    {
        d[i] = (hexd(a[i * 2]) << 4) | hexd(a[i * 2 + 1]);
    }
    vc r(VC_BSTRING, d, (long)len / 2);
    delete [] d;
    return r;
}

vc
vclh_uri_encode(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't uri-encode a non-string", vcnil);
	DwString src((const char *)s, 0, s.len());
	DwString res = UriEncode(src);
	return vc(VC_BSTRING, res.c_str(), res.length());
}

vc
vclh_uri_decode(vc& s)
{
	if(s.type() != VC_STRING)
		USER_BOMB("can't uri-decode a non-string", vcnil);
	DwString src((const char *)s, 0, s.len());
	DwString res = UriDecode(src);
	return vc(VC_BSTRING, res.c_str(), res.length());
}

vc
vclh_set_xferin_constraints(vc& max_memory, vc& max_depth, vc& max_elements, vc& max_element_len)
{
    if(max_memory.type() != VC_INT ||
            max_depth.type() != VC_INT ||
            max_elements.type() != VC_INT ||
            max_element_len.type() != VC_INT)
    {
        USER_BOMB("xferin_constraints args must all be integers (max_memory must be in bytes and > 0, other args -1 means don't change that constraint.)", vcnil);
    }
    vc::set_xferin_constraints(max_memory, max_depth, max_elements, max_element_len);
    return vctrue;
}

vc
dodump()
{
	FILE *f = fopen("dump.out", "a");
	if(!f)
		USER_BOMB("can't open dump.out", vcnil);
	VcIOHack o(f);
	Vcmap->dump(o);
	fclose(f);
	//dump_free();
	return vcnil;
}


vc
dosetdynamic(vc& a)
{
#if 0
	extern int LH_dynamic_binding;
#ifdef CACHE_LOOKUPS
	vc_cvar::flush_lookup_cache();
#endif
	vc_funcall::flush_all_cache();
	LH_dynamic_binding = !a.is_nil();
#endif
	return vcnil;
}

#ifdef OBJTRACK
vc
vclh_dumptrack()
{
	void dump_objmap();
	dump_objmap();
	return vcnil;
}
vc
vclh_clear_objmap()
{
	void clear_objmap();
	clear_objmap();
	return vcnil;
}
#endif

void
vc::setup_logs()
{
	char s1[200];
	char s2[200];

#ifdef __MT__
// note: we don't use vc in thread
// mode any more, can do this later
//#error "do multithreaded logging"
#endif
#ifndef _Windows
	if(getenv("VC_LOG_TO_FILE"))
	{
		sprintf(s1, "lhd%d.out", getpid());
		sprintf(s2, "lho%d.out", getpid());
		freopen(s2, "w", stdout);
		freopen(s1, "w", stderr);
		setbuf(stderr, 0);
	}
#endif
}

void
vc::shutdown_logs()
{
	VcOutput.flush();
	VcError.flush();
}

void
vc::non_lh_init()
{
	vc_winsock::startup();
#ifndef DWYCO_NO_UVSOCK
    DwVP::init_dvp();
    vc_uvsocket::init_uvs_loop();
#endif
}

void
vc::set_xferin_constraints(long max_memory, long max_depth, long max_elements, long max_element_len)
{
    vcxstream::set_default_max_memory(max_memory);
    if(max_depth != -1)
        vcxstream::Max_depth = max_depth;
    if(max_elements != -1)
        vcxstream::Max_elements = max_elements;
    if(max_element_len != -1)
        vcxstream::Max_element_len = max_element_len;
}


// call this once globally before any threads
// are created. this is the usual single-threaded
// init call. DON'T call this in a thread.
void
vc::init()
{
	non_lh_init();
	Vcmap = new vcctx;
	init_rest();
}

#ifndef NO_VCEVAL
static
void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
#endif

#define VC(fun, nicename, attr) vc(fun, nicename, #fun, attr)
#define VC2(fun, nicename, attr, trans) vc(fun, nicename, #fun, attr, trans)
void
vc::init_rest()
{
#ifndef NO_VCEVAL
	// create builtins

	// obscure

	makefun("set-dynamic-binding", VC(dosetdynamic, "set-dynamic-binding", VC_FUNC_BUILTIN_LEAF));

	// function compilation
    
    makefun("compile", VC2(dofundef, "compile", VC_FUNC_BUILTIN_LEAF, trans_compile));
	makefun("lcompile", VC(dolfundef, "lcompile", VC_FUNC_BUILTIN_LEAF));
	makefun("gcompile", VC(dogfundef, "gcompile", VC_FUNC_BUILTIN_LEAF));
	makefun("scompile", VC(dospecial_fundef, "scompile", VC_FUNC_BUILTIN_LEAF));
    makefun("lambda", VC(dolambda, "lambda", VC_FUNC_BUILTIN_LEAF));
    makefun("slambda", VC(doslambda, "slambda", VC_FUNC_BUILTIN_LEAF));

	// meta-data and special function calling
	makefun("get-meta", VC(dofunmeta, "get-meta", VC_FUNC_BUILTIN_LEAF));
	makefun("do-funcall", VC(do_exploded_funcall, "do-funcall", VC_FUNC_BUILTIN_LEAF));
	makefun("type", VC(dotype, "type", VC_FUNC_BUILTIN_LEAF));
	makefun("type2", VC(dotype2, "type2", VC_FUNC_BUILTIN_LEAF));
	makefun("type3", VC(dotype3, "type3", VC_FUNC_BUILTIN_LEAF));

    // control flow
    makefun("return", VC2(doreturn, "return", VC_FUNC_BUILTIN_LEAF, trans_doreturn));
    makefun("if", VC2(doif, "if", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_doif));
    makefun("for", VC2(doloop, "for", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_doloop));
    makefun("while", VC2(dowhile, "while", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_dowhile));
    makefun("break", VC2(dobreak, "break", VC_FUNC_BUILTIN_LEAF, trans_dobreak));
    makefun("cond", VC2(docond, "cond", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_docond));
    makefun("switch", VC2(doswitch, "switch", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_doswitch));
    makefun("cand", VC2(docand, "cand", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_docand));
    makefun("cor", VC2(docor, "cor", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS,trans_docor));

    // arithmetic
	makefun("mod", VC(domod, "mod", VC_FUNC_BUILTIN_LEAF));
    makefun("div", VC2(dodiv, "div", VC_FUNC_BUILTIN_LEAF, trans_dodiv));
    makefun("mul", VC2(domul, "mul", VC_FUNC_BUILTIN_LEAF, trans_domul));
    makefun("add", VC2(doadd, "add", VC_FUNC_BUILTIN_LEAF, trans_doadd));
    makefun("sub", VC2(dosub, "sub", VC_FUNC_BUILTIN_LEAF, trans_dosub));
	makefun("++", VC(incrfun, "++", VC_FUNC_BUILTIN_LEAF));
	makefun("--", VC(decrfun, "--", VC_FUNC_BUILTIN_LEAF));

    // variable binding
	makefun("bind", VC(bindfun, "bind", VC_FUNC_BUILTIN_LEAF));
    makefun("lbind", VC2(lbindfun, "lbind", VC_FUNC_BUILTIN_LEAF, trans_lbindfun));
    makefun("gbind", VC2(gbindfun, "gbind", VC_FUNC_BUILTIN_LEAF, trans_gbindfun));
	makefun("gremove", VC(gremovefun, "gremove", VC_FUNC_BUILTIN_LEAF));
	makefun("lremove", VC(lremovefun, "lremove", VC_FUNC_BUILTIN_LEAF));
	makefun("remove", VC(removefun, "remove", VC_FUNC_BUILTIN_LEAF));
	makefun("bound", VC(boundfun, "bound", VC_FUNC_BUILTIN_LEAF));
	makefun("lbound", VC(lboundfun, "lbound", VC_FUNC_BUILTIN_LEAF));
	makefun("gbound", VC(gboundfun, "gbound", VC_FUNC_BUILTIN_LEAF));
	makefun("gfind", VC(gfindfun, "gfind", VC_FUNC_BUILTIN_LEAF));
	makefun("lfind", VC(lfindfun, "lfind", VC_FUNC_BUILTIN_LEAF));
	// "find" is just too common, maybe need to change the others too
	makefun("vfind", VC(findfun, "vfind", VC_FUNC_BUILTIN_LEAF));
#ifdef LHOBJ
	makefun("oremove", VC(oremovefun, "oremove", VC_FUNC_BUILTIN_LEAF));
	makefun("obind", VC(obindfun, "obind", VC_FUNC_BUILTIN_LEAF));
	makefun("obound", VC(oboundfun, "obound", VC_FUNC_BUILTIN_LEAF));
	makefun("ofind", VC(ofindfun, "ofind", VC_FUNC_BUILTIN_LEAF));
#endif

	// relationals
	makefun("eq", VC(eqfun, "eq", VC_FUNC_BUILTIN_LEAF));
	makefun("ne", VC(nefun, "ne", VC_FUNC_BUILTIN_LEAF));
	makefun("lt", VC(ltfun, "lt", VC_FUNC_BUILTIN_LEAF));
	makefun("le", VC(lefun, "le", VC_FUNC_BUILTIN_LEAF));
	makefun("gt", VC(gtfun, "gt", VC_FUNC_BUILTIN_LEAF));
	makefun("ge", VC(gefun, "ge", VC_FUNC_BUILTIN_LEAF));

    // logical
	makefun("not", VC(notfun, "not", VC_FUNC_BUILTIN_LEAF));
	makefun("or", VC(orfun, "or", VC_FUNC_BUILTIN_LEAF));
	makefun("and", VC(andfun, "and", VC_FUNC_BUILTIN_LEAF));
	makefun("xor", VC(xorfun, "xor", VC_FUNC_BUILTIN_LEAF));
	
    // bitwise
	makefun("bnot", VC(bnotfun, "bnot", VC_FUNC_BUILTIN_LEAF));
	makefun("bor", VC(borfun, "bor", VC_FUNC_BUILTIN_LEAF));
	makefun("band", VC(bandfun, "band", VC_FUNC_BUILTIN_LEAF));
	makefun("bxor", VC(bxorfun, "bxor", VC_FUNC_BUILTIN_LEAF));
	makefun("blsr", VC(blsrfun, "blsr", VC_FUNC_BUILTIN_LEAF));
	makefun("blsl", VC(blslfun, "blsl", VC_FUNC_BUILTIN_LEAF));
	makefun("basr", VC(basrfun, "basr", VC_FUNC_BUILTIN_LEAF));
	makefun("basl", VC(baslfun, "basl", VC_FUNC_BUILTIN_LEAF));

    // set construction
	makefun("list", VC(dolist, "list", VC_FUNC_BUILTIN_LEAF));
	makefun("vector", VC(dovector, "vector", VC_FUNC_BUILTIN_LEAF));
	makefun("vector2", VC(dovectorsize, "vector2", VC_FUNC_BUILTIN_LEAF));
	makefun("vector3", VC(dovector_from_strings, "vector3", VC_FUNC_BUILTIN_LEAF));
	makefun("set", VC(doset, "set", VC_FUNC_BUILTIN_LEAF));
	makefun("set2", VC(doset_size, "set2", VC_FUNC_BUILTIN_LEAF));
	makefun("bag", VC(dobag, "bag", VC_FUNC_BUILTIN_LEAF));
	makefun("bag2", VC(dobag_size, "bag2", VC_FUNC_BUILTIN_LEAF));
	makefun("map", VC(domap, "map", VC_FUNC_BUILTIN_LEAF));
	makefun("tree", VC(create_tree, "tree", VC_FUNC_BUILTIN_LEAF));

	// special atomic construction
	makefun("string", VC(dostring, "string", VC_FUNC_BUILTIN_LEAF));


    // set iteration
    makefun("foreach", VC2(doforeach, "foreach", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS, trans_doforeach));

	// set manipulation
	makefun("append", VC(doappend, "append", VC_FUNC_BUILTIN_LEAF));
	makefun("prepend", VC(doprepend, "prepend", VC_FUNC_BUILTIN_LEAF));
	makefun("put", VC(doaddset, "put", VC_FUNC_BUILTIN_LEAF));
	makefun("get", VC(doget, "get", VC_FUNC_BUILTIN_LEAF));
	makefun("del", VC(doremset, "del", VC_FUNC_BUILTIN_LEAF));
	makefun("del2", VC(doremset2, "del2", VC_FUNC_BUILTIN_LEAF));
	makefun("delfirst", VC(doremfirst, "delfirst", VC_FUNC_BUILTIN_LEAF));
	makefun("delfirst2", VC(doremfirst2, "delfirst2", VC_FUNC_BUILTIN_LEAF));
	makefun("dellast", VC(doremlast, "dellast", VC_FUNC_BUILTIN_LEAF));

    // set query
	makefun("contains", VC(docontains, "contains", VC_FUNC_BUILTIN_LEAF));
	makefun("empty", VC(doempty, "empty", VC_FUNC_BUILTIN_LEAF));
	makefun("numelems", VC(donumelems, "numelems", VC_FUNC_BUILTIN_LEAF));

	// vector ops
	makefun("putidx", VC(doputindex, "putidx", VC_FUNC_BUILTIN_LEAF));
	makefun("getidx", VC(dogetindex, "getidx", VC_FUNC_BUILTIN_LEAF));

	// map ops
	makefun("putkv", VC(doputkv, "putkv", VC_FUNC_BUILTIN_LEAF));

	// exception handling
	makefun("exchandle", VC(doexchandle, "exchandle", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS));
	// new style, kept the old one so as not to break old code.
	// see comments for details, differences are subtle.
	makefun("exchandle2", VC(doexchandle2, "exchandle2", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS));
	makefun("try", VC(dotry, "try", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS));
	makefun("excdhandle", VC(doexcdhandle, "excdhandle", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS));
	makefun("excraise", VC(doexcraise, "excraise", VC_FUNC_BUILTIN_LEAF));
	makefun("excset-handler-ret", VC(dosethandlerret, "excset-handler-ret", VC_FUNC_BUILTIN_LEAF));
	makefun("excbackout", VC(doexcbackout, "excbackout", VC_FUNC_CONSTRUCT|VC_FUNC_DONT_EVAL_ARGS));

	// I/O

    makefun("print", VC(doprint, "print", VC_FUNC_BUILTIN_LEAF));
	makefun("fprint", VC(dofprint, "fprint", VC_FUNC_BUILTIN_LEAF));
	makefun("readline", VC(doreadline, "readline", VC_FUNC_BUILTIN_LEAF));
	makefun("readline2", VC(doreadline2, "readline2", VC_FUNC_BUILTIN_LEAF));
	makefun("readatoms", VC(doreadatoms, "readatoms", VC_FUNC_BUILTIN_LEAF));
	makefun("contents-of", VC(docontents_of, "contents-of", VC_FUNC_BUILTIN_LEAF));
	makefun("flush-std", VC(doflush, "flush-std", VC_FUNC_BUILTIN_LEAF));
    makefun("flush-all", VC(doflushall, "flush-all", VC_FUNC_BUILTIN_LEAF));
	makefun("fgets", VC(dofgets, "fgets", VC_FUNC_BUILTIN_LEAF));
	makefun("fputs", VC(dofputs, "fputs", VC_FUNC_BUILTIN_LEAF));
	makefun("fread", VC(dofread, "fread", VC_FUNC_BUILTIN_LEAF));

	// misc
	
	makefun("gensym", VC(dogensym, "gensym", VC_FUNC_BUILTIN_LEAF));
	makefun("stringrep", VC(dostringrep, "stringrep", VC_FUNC_BUILTIN_LEAF));
	makefun("copy", VC(docopy, "copy", VC_FUNC_BUILTIN_LEAF));
	makefun("void", VC(dovoid, "void", VC_FUNC_BUILTIN_LEAF));
    makefun("prog", VC2(doprog, "prog", VC_FUNC_BUILTIN_LEAF, trans_doprog));
    makefun("eval", VC2(evalfun, "eval", VC_FUNC_BUILTIN_LEAF, trans_eval));

	// string stuff
	makefun("strlen", VC(vclh_strlen, "strlen", VC_FUNC_BUILTIN_LEAF));
	makefun("substr", VC(vclh_substr, "substr", VC_FUNC_BUILTIN_LEAF));
    	makefun("tolower", VC(vclh_tolower, "tolower", VC_FUNC_BUILTIN_LEAF));
    	makefun("strspn", VC(vclh_strspn, "strspn", VC_FUNC_BUILTIN_LEAF));
    	makefun("strcspn", VC(vclh_strcspn, "strcspn", VC_FUNC_BUILTIN_LEAF));
    	makefun("strstr", VC(vclh_strstr, "strstr", VC_FUNC_BUILTIN_LEAF));

	// regular expression handling
#ifndef DWYCO_NO_VCREGEX
	makefun("regex", VC(doregex, "regex", VC_FUNC_BUILTIN_LEAF));
	makefun("split", VC(doresplit, "split", VC_FUNC_BUILTIN_LEAF));
	makefun("match", VC(dorematch, "match", VC_FUNC_BUILTIN_LEAF));
	//makefun("search", VC(doresearch, "search", VC_FUNC_BUILTIN_LEAF));
	//makefun("search2", VC(doresearch2, "search2", VC_FUNC_BUILTIN_LEAF));
#endif

	// files

	makefun("openfile", VC(doopenfile, "openfile", VC_FUNC_BUILTIN_LEAF));
	makefun("closefile", VC(doclosefile, "closefile", VC_FUNC_BUILTIN_LEAF));
	makefun("seekfile", VC(doseekfile, "seekfile", VC_FUNC_BUILTIN_LEAF));

	// debugging
	makefun("backtrace", VC(dobacktrace, "backtrace", VC_FUNC_BUILTIN_LEAF));
	makefun("dump", VC(dodump, "dump", VC_FUNC_BUILTIN_LEAF));

	// process control
	 makefun("exit", VC(doexit, "exit", VC_FUNC_BUILTIN_LEAF));
#if 0
    // sockets
	makefun("socket", VC(lh_socket, "socket", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-from-os-handle", VC(lh_socket_from_os_handle, "socket-from-os-handle", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-os-handle", VC(lh_horrible_hack, "socket-os-handle", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-close", VC(lh_sockclose, "socket-close", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-shutdown", VC(lh_sockshutdown, "socket-shutdown", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-accept", VC(lh_accept, "socket-accept", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-connect", VC(lh_connect, "socket-connect", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-poll", VC(lh_poll, "socket-poll", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-poll-all", VC(lh_poll_all, "socket-poll-all", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-poll-sets", VC(lh_poll_sets, "socket-poll-sets", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-add-write-set", VC(lh_add_write_set, "socket-add-write-set", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-add-read-set", VC(lh_add_read_set, "socket-add-read-set", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-del-write-set", VC(lh_del_write_set, "socket-del-write-set", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-del-read-set", VC(lh_del_read_set, "socket-del-read-set", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-clear-read-set", VC(lh_clear_read_set, "socket-clear-read-set", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-clear-write-set", VC(lh_clear_write_set, "socket-clear-write-set", VC_FUNC_BUILTIN_LEAF));

	makefun("socket-send", VC(lh_socksend, "socket-send", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-recv", VC(lh_sockrecv, "socket-recv", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-send-str", VC(lh_socksendstring, "socket-send-str", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-recv-str", VC(lh_sockrecvstring, "socket-recv-str", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-send-buf", VC(lh_socksend_buf, "socket-send-buf", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-recv-buf", VC(lh_sockrecv_buf, "socket-recv-buf", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-set-option", VC(lh_sockset_option, "socket-set-option", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-peer-addr", VC(lh_sock_peer_addr, "socket-peer-addr", VC_FUNC_BUILTIN_LEAF));
	makefun("socket-local-addr", VC(lh_sock_local_addr, "socket-local-addr", VC_FUNC_BUILTIN_LEAF));
	
	makefun("gethostbyname", VC(lh_gethostbyname, "gethostbyname", VC_FUNC_BUILTIN_LEAF));

#ifndef DWYCO_NO_UVSOCK
    makefun("uv-socket", VC(lh_uv_socket, "uv-socket", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-sockclose", VC(lh_uv_sockclose, "uv-sockclose", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-sockshutdown", VC(lh_uv_sockshutdown, "uv-sockshutdown", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-connect", VC(lh_uv_connect, "uv-connect", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-poll", VC(lh_uv_poll, "uv-poll", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-socksend", VC(lh_uv_socksend, "uv-socksend", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-sockrecv", VC(lh_uv_sockrecv, "uv-sockrecv", VC_FUNC_BUILTIN_LEAF));
    makefun("uv-get-all", VC(lh_uv_get_all, "uv-get-all", VC_FUNC_BUILTIN_LEAF));
#endif
#endif
#ifdef LHOBJ
	makefun("make-factory", VC(vclh_make_factory, "make-factory", VC_FUNC_BUILTIN_LEAF));
	makefun("lmake-factory", VC(vclh_lmake_factory, "lmake-factory", VC_FUNC_BUILTIN_LEAF));
	makefun("gmake-factory", VC(vclh_gmake_factory, "gmake-factory", VC_FUNC_BUILTIN_LEAF));

#endif
#if 0
	// math functions
	makefun("sqrt", VC(vcsqrt, "sqrt", VC_FUNC_BUILTIN_LEAF));
	makefun("sin", VC(vcsin, "sin", VC_FUNC_BUILTIN_LEAF));
	makefun("cos", VC(vccos, "cos", VC_FUNC_BUILTIN_LEAF));
	makefun("tan", VC(vctan, "tan", VC_FUNC_BUILTIN_LEAF));
	makefun("abs", VC(vcabs, "abs", VC_FUNC_BUILTIN_LEAF));
	makefun("exp", VC(vcexp, "exp", VC_FUNC_BUILTIN_LEAF));
    makefun("ln", VC(vclog, "ln", VC_FUNC_BUILTIN_LEAF));
	makefun("log10", VC(vclog10, "log10", VC_FUNC_BUILTIN_LEAF));
	makefun("pow", VC(vcpow, "pow", VC_FUNC_BUILTIN_LEAF));
	makefun("acos", VC(vcacos, "acos", VC_FUNC_BUILTIN_LEAF));
	makefun("asin", VC(vcasin, "asin", VC_FUNC_BUILTIN_LEAF));
	makefun("atan", VC(vcatan, "atan", VC_FUNC_BUILTIN_LEAF));
	makefun("atan2", VC(vcatan2, "atan2", VC_FUNC_BUILTIN_LEAF));
	makefun("floor", VC(vcfloor, "floor", VC_FUNC_BUILTIN_LEAF));
	makefun("atof", VC(vcatof, "atof", VC_FUNC_BUILTIN_LEAF));
	makefun("hypot", VC(vchypot, "hypot", VC_FUNC_BUILTIN_LEAF));
	// simple linear searches of a vector
	makefun("findmin", VC(vcfindmin, "findmin", VC_FUNC_BUILTIN_LEAF));
	makefun("findmax", VC(vcfindmax, "findmax", VC_FUNC_BUILTIN_LEAF));
	makefun("findeq", VC(vcfindeq, "findeq", VC_FUNC_BUILTIN_LEAF));
	makefun("findne", VC(vcfindne, "findne", VC_FUNC_BUILTIN_LEAF));
	// summing vectors
	makefun("sum", VC(vclhsum, "sum", VC_FUNC_BUILTIN_LEAF));
	makefun("sum2", VC(vclhsum2, "sum2", VC_FUNC_BUILTIN_LEAF));
	// random numbers
	makefun("rand", VC(vcrand, "rand", VC_FUNC_BUILTIN_LEAF));
	makefun("srand", VC(vcsrand, "srand", VC_FUNC_BUILTIN_LEAF));

#endif
	// system related functions
	makefun("getenv", VC(vclh_getenv, "getenv", VC_FUNC_BUILTIN_LEAF));
	makefun("putenv", VC(vclh_putenv, "putenv", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-file-size", VC(vclh_file_size, "fs-file-size", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-file-exists", VC(vclh_file_exists, "fs-file-exists", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-file-access", VC(vclh_file_access, "fs-file-access", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-file-remove", VC(vclh_file_remove, "fs-file-remove", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-dir-to-map", VC(vclh_dir2map, "fs-dir-to-map", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-file-rename", VC(vclh_file_rename, "fs-file-rename", VC_FUNC_BUILTIN_LEAF));
	makefun("fs-file-stat", VC(vclh_file_stat, "fs-file-stat", VC_FUNC_BUILTIN_LEAF));
	makefun("time", VC(vclh_time, "time", VC_FUNC_BUILTIN_LEAF));
	makefun("time-hp", VC(vclh_time_hp, "time-hp", VC_FUNC_BUILTIN_LEAF));
	makefun("time-hp2", VC(vclh_time_hp2, "time-hp2", VC_FUNC_BUILTIN_LEAF));
	makefun("strftime", VC(vclh_strftime, "strftime", VC_FUNC_BUILTIN_LEAF));
	makefun("strftime-hp", VC(vclh_strftime_hp, "strftime-hp", VC_FUNC_BUILTIN_LEAF));
	makefun("sleep", VC(vclh_sleep, "sleep", VC_FUNC_BUILTIN_LEAF));
	makefun("system", VC(vclh_system, "system", VC_FUNC_BUILTIN_LEAF));
#ifdef PARSEDATE
	makefun("parsedate", VC(vclh_parsedate, "parsedate", VC_FUNC_BUILTIN_LEAF));
#endif
	makefun("process-create", VC(vclh_process_create, "process-create", VC_FUNC_BUILTIN_LEAF));
	makefun("process-clean-zombies", VC(vclh_clean_zombies, "process-clean-zombies", VC_FUNC_BUILTIN_LEAF));
	makefun("process-clean-zombies2", VC(vclh_clean_zombies2, "process-clean-zombies2", VC_FUNC_BUILTIN_LEAF));
	makefun("process-kill", VC(vclh_kill, "process-kill", VC_FUNC_BUILTIN_LEAF));
	makefun("process-alarm", VC(vclh_alarm, "process-alarm", VC_FUNC_BUILTIN_LEAF));

	// simple formatting
	makefun("fmt", VC(vclh_fmt, "fmt", VC_FUNC_BUILTIN_LEAF));
	makefun("atol", VC(vclh_atol, "atol", VC_FUNC_BUILTIN_LEAF));
	makefun("to-hex", VC(vclh_to_hex, "to-hex", VC_FUNC_BUILTIN_LEAF));
	makefun("from-hex", VC(vclh_from_hex, "from-hex", VC_FUNC_BUILTIN_LEAF));

	// saving/restoring xfer format 
	makefun("putfile", VC(doputfile, "putfile", VC_FUNC_BUILTIN_LEAF));
    makefun("getfile", VC(dogetfile, "getfile", VC_FUNC_BUILTIN_LEAF));
	makefun("serialize", VC(vclh_serialize, "serialize", VC_FUNC_BUILTIN_LEAF));
	makefun("deserialize", VC(vclh_deserialize, "deserialize", VC_FUNC_BUILTIN_LEAF));

	makefun("uri-encode", VC(vclh_uri_encode, "uri-encode", VC_FUNC_BUILTIN_LEAF));
	makefun("uri-decode", VC(vclh_uri_decode, "uri-decode", VC_FUNC_BUILTIN_LEAF));
#if 0
#ifndef NO_LHCRYPTO
	makefun("SHA3_256", VC(vclh_sha3_256, "SHA3_256", VC_FUNC_BUILTIN_LEAF));
	makefun("SHA256", VC(vclh_sha256, "SHA256", VC_FUNC_BUILTIN_LEAF));
	makefun("SHA", VC(vclh_sha, "SHA", VC_FUNC_BUILTIN_LEAF));
	makefun("MD5", VC(vclh_md5, "MD5", VC_FUNC_BUILTIN_LEAF));
	makefun("base64-encode", VC(vclh_base64_encode, "base64-encode", VC_FUNC_BUILTIN_LEAF));
	makefun("base64-decode", VC(vclh_base64_decode, "base64-decode", VC_FUNC_BUILTIN_LEAF));
	makefun("DH-init", VC(vclh_dh_init, "DH-init", VC_FUNC_BUILTIN_LEAF));
	makefun("DH-save", VC(vclh_dh_save, "DH-save", VC_FUNC_BUILTIN_LEAF));
	makefun("DH-setup", VC(vclh_dh_setup, "DH-setup", VC_FUNC_BUILTIN_LEAF));
	makefun("DH-agree", VC(vclh_dh_agree, "DH-agree", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-init", VC(lh_bf_init, "BF-init", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-init-key-stream", VC(lh_bf_init_key_stream, "BF-init-key-stream", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-init-key-cbc", VC(lh_bf_init_key_cbc, "BF-init-key-cbc", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-init-key-ecb", VC(lh_bf_init_key, "BF-init-key-ecb", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-enc", VC(lh_bf_enc, "BF-enc", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-dec", VC(lh_bf_dec, "BF-dec", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-xfer-enc", VC(lh_bf_xfer_enc, "BF-xfer-enc", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-xfer-dec", VC(lh_bf_xfer_dec, "BF-xfer-dec", VC_FUNC_BUILTIN_LEAF));

	makefun("BF-open-ctx", VC(vclh_bf_open, "BF-open-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-close-ctx", VC(vclh_bf_close, "BF-close-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-init-key-stream-ctx", VC(vclh_bf_init_key_stream_ctx, "BF-init-key-stream-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-init-key-cbc-ctx", VC(vclh_bf_init_key_cbc_ctx, "BF-init-key-cbc-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-enc-ctx", VC(vclh_bf_enc_ctx, "BF-enc-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-dec-ctx", VC(vclh_bf_dec_ctx, "BF-dec-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-xfer-dec-ctx", VC(vclh_bf_xfer_dec_ctx, "BF-xfer-dec-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("BF-xfer-enc-ctx", VC(vclh_bf_xfer_enc_ctx, "BF-xfer-enc-ctx", VC_FUNC_BUILTIN_LEAF));


	makefun("GCM-open-ctx", VC(vclh_encdec_open, "GCM-open-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("GCM-close-ctx", VC(vclh_encdec_close, "GCM-close-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("GCM-init-key-ctx", VC(vclh_encdec_init_key_ctx, "GCM-init-key-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("GCM-enc-ctx", VC(vclh_encdec_enc_ctx, "GCM-enc-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("GCM-dec-ctx", VC(vclh_encdec_dec_ctx, "GCM-dec-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("GCM-xfer-dec-ctx", VC(vclh_encdec_xfer_dec_ctx, "GCM-xfer-dec-ctx", VC_FUNC_BUILTIN_LEAF));
	makefun("GCM-xfer-enc-ctx", VC(vclh_encdec_xfer_enc_ctx, "GCM-xfer-enc-ctx", VC_FUNC_BUILTIN_LEAF));

	makefun("DSA-init", VC(vclh_dsa_init, "DSA-init", VC_FUNC_BUILTIN_LEAF));
	makefun("DSA-save", VC(vclh_dsa_save, "DSA-save", VC_FUNC_BUILTIN_LEAF));
	makefun("DSA-init-pub", VC(vclh_dsa_pub_init, "DSA-init-pub", VC_FUNC_BUILTIN_LEAF));
	makefun("DSA-sign", VC(vclh_dsa_sign, "DSA-sign", VC_FUNC_BUILTIN_LEAF));
	makefun("DSA-verify", VC(vclh_dsa_verify, "DSA-verify", VC_FUNC_BUILTIN_LEAF));
    makefun("UDH-init", VC(udh_init, "UDH-init", VC_FUNC_BUILTIN_LEAF));
    makefun("UDH-new-static", VC(udh_new_static, "UDH-new-static", VC_FUNC_BUILTIN_LEAF));
    makefun("UDH-gen-keys", VC(udh_gen_keys, "UDH-gen-keys", VC_FUNC_BUILTIN_LEAF));
    makefun("UDH-just-publics", VC(udh_just_publics, "UDH-just-publics", VC_FUNC_BUILTIN_LEAF));
    makefun("UDH-agree", VC(udh_agree_auth, "UDH-agree", VC_FUNC_BUILTIN_LEAF));

    // WARNING: these functions use the old single-key API for these functions
    // this is used for setting up simple GCM encryption with clients of cdc-x
    // servers. also used for some group-entry protocol stuff.
    makefun("UDH-sf-material", VC(vclh_sf_material, "UDH-sf-material", VC_FUNC_BUILTIN_LEAF));
    makefun("UDH-sf-get-key", VC(vclh_dh_store_and_forward_get_key, "UDH-sf-get-key", VC_FUNC_BUILTIN_LEAF));
#endif
	makefun("GZ-compress-open", VC(vclh_compression_open, "GZ-compress-open", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-compress-close", VC(vclh_compression_close, "GZ-compress-close", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-decompress-open", VC(vclh_decompression_open, "GZ-decompress-open", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-decompress-close", VC(vclh_decompression_close, "GZ-decompress-close", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-compress", VC(vclh_compress, "GZ-compress", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-decompress", VC(vclh_decompress, "GZ-decompress", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-compress-xfer", VC(vclh_compress_xfer, "GZ-compress-xfer", VC_FUNC_BUILTIN_LEAF));
	makefun("GZ-decompress-xfer", VC(lh_decompress_xfer, "GZ-decompress-xfer", VC_FUNC_BUILTIN_LEAF));

    makefun("set-xferin-constraints", VC(vclh_set_xferin_constraints, "set-xferin-constraints", VC_FUNC_BUILTIN_LEAF));
	// debugging
#ifdef VCDBG
	makefun("__lh_break_on_call", VC(vclh_break_on_call, "__lh_break_on_call", VC_FUNC_BUILTIN_LEAF));
	makefun("__lh_break_on_access", VC(vclh_break_on_access, "__lh_break_on_access", VC_FUNC_BUILTIN_LEAF));
	makefun("__lh_eval_break_on", VC(vclh_eval_break_on, "__lh_eval_break_on", VC_FUNC_BUILTIN_LEAF));
	makefun("__lh_eval_break_off", VC(vclh_eval_break_off, "__lh_eval_break_off", VC_FUNC_BUILTIN_LEAF));
#endif
#ifdef OBJTRACK
	makefun("__lh_dumptrack", VC(vclh_dumptrack, "__lh_dumptrack", VC_FUNC_BUILTIN_LEAF));
	makefun("__lh_clear_objmap", VC(vclh_clear_objmap, "__lh_clear_objmap", VC_FUNC_BUILTIN_LEAF));
#endif
	makefun("dump-flat-profile", VC(dump_flat_profile, "dump-flat-profile", VC_FUNC_BUILTIN_LEAF));

#ifdef LH_WRAP_SQLITE3
void wrapper_init_sqlite3();
wrapper_init_sqlite3();
#endif
#ifdef VCPCRE
void init_vcpcre();
init_vcpcre();
#endif
#ifdef LH_WRAP_SPREAD
void wrapper_init_spread_xml();
wrapper_init_spread_xml();
#endif
#endif

#endif // NO_VCEVAL

}

void
vc::exit()
{
	VcOutput.flush();
	VcError.flush();
	delete Vcmap;
	Vcmap = 0;
	non_lh_exit();
}


void
vc::non_lh_exit()
{
#ifndef DWYCO_NO_UVSOCK
    vc_uvsocket::exit_uvs_loop();
#endif
	vc_winsock::shutoff();
}
