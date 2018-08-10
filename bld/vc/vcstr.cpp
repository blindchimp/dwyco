
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <ctype.h>
#include "vc.h"
#include "vcstr.h"
#include "vcmap.h"
#include "vcxstrm.h"
#include "vcenco.h"
#include <new>
#include "jhash.h"
#include "vcio.h"

using namespace std;
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcstr.cpp 1.51 1998/12/09 05:12:24 dwight Exp $";

hashValueType
vc_string::hashstr() const
{
	return jenkins_hashlittle(str, cached_len, 0);
}

void
vc_string::cache_hash() const
{
	cached_hv = hashstr();
	is_cached = 1;
}

void
vc_string::global_bind(const vc& v) const
{
	if(str[0] == '\0' && cached_len == 0)
		USER_BOMB2("binding to zero length string not allowed");
	vc lhs(str);
	Vcmap->global_add(lhs, v);
}

void
vc_string::local_bind(const vc& v) const
{
	if(str[0] == '\0' && cached_len == 0)
		USER_BOMB2("binding to zero length string not allowed");
	vc lhs(str);
	Vcmap->local_add(lhs, v);
}

void
vc_string::bind(const vc& v) const
{
	if(str[0] == '\0' && cached_len == 0)
		USER_BOMB2("binding to zero length string not allowed");
	vc lhs(str);
	Vcmap->add(lhs, v);
}

void
vc_string::local_bremove() const
{
	if(str[0] == '\0' && cached_len == 0)
		USER_BOMB2("removing zero length string not allowed");
	vc lhs(str);
	Vcmap->local_remove(lhs);
}

void
vc_string::global_bremove() const
{
	if(str[0] == '\0' && cached_len == 0)
		USER_BOMB2("removing zero length string not allowed");
	vc lhs(str);
	Vcmap->global_remove(lhs);
}

void
vc_string::bremove() const
{
	if(str[0] == '\0' && cached_len == 0)
		USER_BOMB2("removing zero length string not allowed");
	vc lhs(str);
	Vcmap->remove(lhs);
}

vc
vc_string::eval() const
{
    oopanic("string eval?");
    //return Vcmap->get(vc(VC_STRING, str, cached_len));
    return vcnil;
}

void
vc_string::stringrep(VcIO o) const
{
	o << "|";
	for(int i = 0; i < cached_len; ++i)
    {
		if(str[i] == '|' || str[i] == '\\')
			o << '\\';
		o << str[i];
    }
	o << "|";
}

vc_string::vc_string(VcNoinit)
{
	str = 0;
	cached_hv = 0;
	cached_len = 0;
	is_cached = 0;
}

vc_string::vc_string(const char *s)
{
	cached_len = strlen(s);
	str = new char[cached_len + 1];
	strcpy(str, s);
	is_cached = 0;
}

// this one is geared towards strings that might
// contain nulls, but tries to keep compatibility
// with other strings by putting null termination in.
vc_string::vc_string(const char *s, int len)
{
	cached_len = len;
	// +1 for zero termination (compatibility)
	str = new char[cached_len + 1];
	memcpy(str, s, len);
	str[len] = '\0';
	is_cached = 0;
}

vc_string::vc_string(const vc_string& v)
{
	str = new char[v.cached_len + 1];
	memcpy(str, v.str, v.cached_len + 1);
	cached_hv = v.cached_hv;
	cached_len = v.cached_len;
	is_cached = v.is_cached;
}

vc_default *
vc_string::do_copy() const
{
	return new vc_string(*this);
}

vc_string:: ~vc_string()
{
	delete [] str;
}


vc_string::operator const char *() const {return str;}
vc_string::operator int () const {return atoi(str); }
vc_string::operator double() const {return atof(str); }
vc_string::operator long() const { return atol(str); }

const char *
vc_string::peek_str() const { return str;}
long
vc_string::len() const {return cached_len;}

void
vc_string::bomb(void) const {USER_BOMB2("can't compute on strings"); }
void
vc_string::bomb_op(void) const {USER_BOMB2("can't compare non-numeric with arithmetic ops"); }
vc
vc_string::operator+(const vc &v) const {bomb(); return v;}
vc
vc_string::operator-(const vc &v) const {bomb(); return v;}
vc
vc_string::operator*(const vc &v) const {bomb(); return v;}
vc
vc_string::operator/(const vc &v) const {bomb(); return v;}
vc
vc_string::operator%(const vc &v) const {bomb(); return v;}

enum vc_type
vc_string::type() const { return VC_STRING; }
int
vc_string::is_nil() const { 
return 0;
//return strncmp(str, "nil", 4) == 0; 
}

int
vc_string::operator <(const vc &v) const { return v.str_lt(*this); }
int
vc_string::operator <=(const vc &v) const {return v.str_le(*this); }
int
vc_string::operator >(const vc &v) const { return v.str_gt(*this); }
int
vc_string::operator >=(const vc &v) const {return v.str_ge(*this); }
int
vc_string::operator ==(const vc &v) const {return v.str_eq(*this); }
int
vc_string::operator !=(const vc &v) const {return v.str_ne(*this); }

// in keeping with the convenience hack in vcfuncal.cpp, we
// allowing calling of strings by mapping first...
// note: this allows a tight infinite recursion:
// if you map a a string to itself, and then funcall it,
// it crashes the system, pronto.
// 
vc
vc_string::operator()(void) const {return eval()();}
//vc
//vc_string::operator()(void *p) const {return eval()(p);}
vc
vc_string::operator()(VCArglist *al) const {return eval()(al);}

vc
vc_string::operator()(vc v0) const {return eval()(v0);}
vc
vc_string::operator()(vc v0, vc v1) const {return eval()(v0, v1);}
vc
vc_string::operator()(vc v0, vc v1, vc v2) const {return eval()(v0, v1, v2);}
vc
vc_string::operator()(vc v0, vc v1, vc v2, vc v3) const {return eval()(v0, v1, v2, v3);}

int
vc_string::int_lt(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::int_le(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::int_gt(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::int_ge(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::int_eq(const vc&) const {return  FALSE;}
int
vc_string::int_ne(const vc&) const {return  TRUE;}

int
vc_string::double_lt(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::double_le(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::double_gt(const vc&) const  {bomb_op(); return FALSE;}
int
vc_string::double_ge(const vc&) const {bomb_op(); return FALSE;}
int
vc_string::double_eq(const vc&) const {return  FALSE;}
int
vc_string::double_ne(const vc&) const {return  TRUE;}

int
vc_string::str_eq(const vc& v) const
{
	if(!is_cached)
		cache_hash();
	if(!((const vc_string&)v).is_cached)
		((const vc_string&)v).cache_hash();
	if(cached_hv != ((const vc_string&)v).cached_hv
		|| cached_len != ((const vc_string&)v).cached_len)
		return 0;
	return memcmp(str, ((const vc_string&)v).str, cached_len) == 0;
}

int
vc_string::str_ne(const vc& v) const
{
	if(!is_cached)
		cache_hash();
	if(!((const vc_string&)v).is_cached)
		((const vc_string&)v).cache_hash();
	if(cached_hv != ((const vc_string&)v).cached_hv
		|| cached_len != ((const vc_string&)v).cached_len)
		return 1;
	return memcmp(str, ((const vc_string&)v).str, cached_len) != 0;
}

int
vc_string::str_le(const vc& v) const {
	const vc_string& v2 = (const vc_string&)v;
	int res = memcmp(v2.str, str, dwmin(v2.cached_len, cached_len));
	if(res == 0)
	{
		return v2.cached_len <= cached_len;
	}
	return res < 0;
}
int
vc_string::str_lt(const vc& v) const {
	const vc_string& v2 = (const vc_string&)v;
	int res = memcmp(v2.str, str, dwmin(v2.cached_len, cached_len));
	if(res == 0)
	{
		return v2.cached_len < cached_len;
	}
	return res < 0;
}
int
vc_string::str_ge(const vc& v) const {
	const vc_string& v2 = (const vc_string&)v;
	int res = memcmp(v2.str, str, dwmin(v2.cached_len, cached_len));
	if(res == 0)
	{
		return v2.cached_len >= cached_len;
	}
	return res > 0;
}
int
vc_string::str_gt(const vc& v) const {
	const vc_string& v2 = (const vc_string&)v;
	int res = memcmp(v2.str, str, dwmin(v2.cached_len, cached_len));
	if(res == 0)
	{
		return v2.cached_len > cached_len;
	}
	return res > 0;
}

hashValueType
vc_string::hashValue() const
{
	if(!is_cached)
		cache_hash();
	return cached_hv;
}

void
vc_string::printOn(VcIO outputStream)
{
	
	//outputStream << str;
	for(int i = 0; i < cached_len; ++i)
	{
        if(!isprint(str[i] & 0xff) && !isspace(str[i] & 0xff))
		{
			outputStream << "\\x";
			outputStream.set_format("%02x");
			outputStream << (unsigned char)(str[i]);
			outputStream.set_format(0);
		}
		else 
		{
			outputStream << str[i];
		}
	}
}

int
vc_string::func_eq(const vc&) const {bomb_op_func(); return 0;}

long
vc_string::xfer_out(vcxstream& vcx)
{
	char buf[40];
    long slen = cached_len;
	int len = encode_long(buf, slen);
	char *cp;
	if((cp = vcx.out_want(len + slen)) == 0)
		return -1;
	memcpy(cp, buf, len);
	memcpy(cp + len, str, slen);
    return len + slen;
}

long
vc_string::xfer_in(vcxstream& vcx)
{
	char *cp;
	if((cp = vcx.in_want(ENCODED_LEN_LEN)) == 0)
		return EXIN_DEV;
	long len = decode_len(cp);
    if(len == -1 || len == 0)
		return EXIN_PARSE;
    if(len > vcx.max_element_len)
        return EXIN_PARSE;
	if((cp = vcx.in_want(len)) == 0)
		return EXIN_DEV;
	long slen = decode_long(cp, len);
	if(slen == -1)
		return EXIN_PARSE;
    if(slen > vcx.max_element_len)
        return EXIN_PARSE;

    char *ostr = str;
    // do this in case slen is wildly big
#ifndef ANDROID
    new_handler nh = set_new_handler(0);
#endif
	str = new char [slen + 1]; // +1 for 0 termination
	if(str == 0)
	{
		str = ostr;
#ifndef ANDROID
		set_new_handler(nh);
#endif
		return EXIN_PARSE;
	}
	delete [] ostr;
#ifndef ANDROID
	set_new_handler(nh);
#endif
	str[0] = '\0'; // in case of failure later
    cached_len = 0;
	if((cp = vcx.in_want(slen)) == 0)
		return EXIN_DEV;
	memcpy(str, cp, slen);
	str[slen] = '\0';
	cached_len = slen;
	is_cached = 0;
	return ENCODED_LEN_LEN + len + slen;
}
