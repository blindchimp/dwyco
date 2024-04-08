
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdio.h>
#include "vc.h"
#include "vcdbl.h"
#include "vcint.h"
#include "vcdecom.h"
#include "vcmap.h"
#include "vcenco.h"
#include "vcio.h"
#include "vcxstrm.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcdbl.cpp 1.49 1998/12/09 05:12:34 dwight Exp $";

// this is bogus and needs to be gotten rid of
char vc_double::buf[2048];

vc_double::vc_double() { d = 0.0;  }
vc_double::vc_double(double d2) { d = d2;  }
vc_double::vc_double(const vc_int &v) { d = v.i; }
vc_double::vc_double(const vc_double &v) { d = v.d; }
vc_double::~vc_double() { }

vc_double::operator double() const {return d; }
vc_double::operator int() const {USER_BOMB("truncating double", 0);}
vc_double::operator long() const {USER_BOMB("truncating double", 0);}
vc_double::operator const char *() const {USER_BOMB("can't convert double to char *", "0");}

const char *
vc_double::peek_str() const {
    int ret = snprintf(buf, sizeof(buf), "%g", d);
    if(ret < 0 || ret + 1 >= sizeof(buf))
        oopanic("double peek programming error");
    return buf;
}

void
vc_double::stringrep(VcIO o) const {o << d;}

vc
vc_double::operator+(const vc &v) const {return v.double_add(*this);}
vc
vc_double::operator-(const vc &v) const {return v.double_sub(*this);}
vc
vc_double::operator*(const vc &v) const {return v.double_mul(*this);}
vc
vc_double::operator/(const vc &v) const {return v.double_div(*this);}
vc
vc_double::operator%(const vc &v) const {return v.double_mod(*this);}


#define PROLOG     vc retval;vc_double *v1 = new vc_double(((const vc_int &)v));
#define EPILOG   retval.redefine(v1);return retval;
vc
vc_double::int_add(const vc& v) const {PROLOG; v1->d += d;EPILOG}
vc
vc_double::int_sub(const vc& v) const {PROLOG; v1->d -= d;EPILOG}
vc
vc_double::int_mul(const vc& v) const {PROLOG; v1->d *= d;EPILOG}
vc
vc_double::int_div(const vc& v) const {PROLOG; v1->d /= d;EPILOG}
vc
vc_double::int_mod(const vc& v) const {USER_BOMB("can't modulo floating numbers", v); }

#undef PROLOG
#undef EPILOG

#define PROLOG     vc retval;vc_double *v1 = new vc_double((const vc_double &)v);
#define EPILOG   retval.redefine(v1);return retval;
vc
vc_double::double_add(const vc& v) const {PROLOG; v1->d += d;EPILOG}
vc
vc_double::double_sub(const vc& v) const {PROLOG; v1->d -= d;EPILOG}
vc
vc_double::double_mul(const vc& v) const {PROLOG; v1->d *= d;EPILOG}
vc
vc_double::double_div(const vc& v) const {PROLOG; v1->d /= d;EPILOG}
vc
vc_double::double_mod(const vc& v) const {USER_BOMB("can't modulo floating numbers",v); }

#undef PROLOG
#undef EPILOG

#define PROLOG     vc retval;vc_vector *v1 = new vc_vector(((const vc_vector&)v).num_elems());
#define EPILOG   retval.redefine(v1);return retval;
vc vc_double::vec_add(const vc& v) const {PROLOG; v1->scal1_add((const vc_vector &)v, *this); EPILOG}
vc vc_double::vec_sub(const vc& v) const {PROLOG; v1->scal1_sub((const vc_vector &)v, *this);EPILOG}
vc vc_double::vec_mul(const vc& v) const {PROLOG; v1->scal1_mul((const vc_vector &)v, *this);EPILOG}
vc vc_double::vec_div(const vc& v) const {PROLOG; v1->scal1_div((const vc_vector &)v, *this);EPILOG}
vc vc_double::vec_mod(const vc& v) const {PROLOG; v1->scal1_mod((const vc_vector &)v, *this);EPILOG}

#undef PROLOG
#undef EPILOG

int
vc_double::int_lt(const vc& v) const {return ((const vc_int&)v).i <  d;}
int
vc_double::int_le(const vc& v) const {return  ((const vc_int&)v).i <= d;}
int
vc_double::int_gt(const vc& v) const {return ((const vc_int&)v).i >  d;}
int
vc_double::int_ge(const vc& v) const {return  ((const vc_int&)v).i >= d;}
int
vc_double::int_eq(const vc& v) const {return  ((const vc_int&)v).i == d;}
int
vc_double::int_ne(const vc& v) const {return  ((const vc_int&)v).i != d;}

int
vc_double::double_lt(const vc& v) const {return ((const vc_double&)v).d <  d;}
int
vc_double::double_le(const vc& v) const {return  ((const vc_double&)v).d <= d;}
int
vc_double::double_gt(const vc& v) const {return ((const vc_double&)v).d >  d;}
int
vc_double::double_ge(const vc& v) const {return  ((const vc_double&)v).d >= d;}
int
vc_double::double_eq(const vc& v) const {return  ((const vc_double&)v).d == d;}
int
vc_double::double_ne(const vc& v) const {return  ((const vc_double&)v).d != d;}

enum vc_type
vc_double::type() const { return VC_DOUBLE; }

int
vc_double::operator <(const vc &v) const {  return v.double_lt(*this); }
int
vc_double::operator <=(const vc &v) const {  return v.double_le(*this); }
int
vc_double::operator >(const vc &v) const {  return v.double_gt(*this); }
int
vc_double::operator >=(const vc &v) const {  return v.double_ge(*this); }
int
vc_double::operator ==(const vc &v) const {  return v.double_eq(*this); }
int
vc_double::operator !=(const vc &v) const {  return v.double_ne(*this); }

hashValueType
vc_double::hashValue() const {return (hashValueType)(d * 100.);}
void
vc_double::printOn(VcIO outputStream) {outputStream << d;}

long
vc_double::xfer_out(vcxstream& vcx)
{
	char *cp;
	char buf[40];
	char fbuf[2048];

    snprintf(fbuf, sizeof(fbuf), "%.*g", (int)(sizeof(fbuf) / 2), d);

	int flen = strlen(fbuf);
	int len = encode_long(buf, flen);
	if((cp = vcx.out_want(flen + len)) == 0)
		return -1;
	memcpy(cp, buf, len);
	memcpy(cp + len, fbuf, flen);
    return len + flen;
}

long
vc_double::xfer_in(vcxstream& vcx)
{
	char *cp;

	if((cp = vcx.in_want(ENCODED_LEN_LEN)) == 0)
		return EXIN_DEV;
    int len = decode_len(cp);
    if(len == -1 || len == 0 || len > vcx.max_count_digits)
		return EXIN_PARSE;
	if((cp = vcx.in_want(len)) == 0)
		return EXIN_DEV;
	long flen = decode_long(cp, len);
    // we know length has to be > 0
    if(flen == -1 || flen == 0)
		return EXIN_PARSE;
    if(flen > vcx.max_element_len)
    {
        user_warning("xfer_in double hit max_element len");
        return EXIN_PARSE;
    }
	if((cp = vcx.in_want(flen)) == 0)
		return EXIN_DEV;

	char *end;
	char buf[2048];

	if(flen - 1 >= sizeof(buf))
		return EXIN_PARSE;
	memcpy(buf, cp, flen);
    buf[flen] = '\0';
	d = strtod(buf, &end);
	if(end != buf + flen)
		return EXIN_PARSE;
    return ENCODED_LEN_LEN + len + flen;
}
