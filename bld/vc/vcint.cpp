
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#if defined(LINUX) || defined(_MSC_VER)
#include <limits.h>
#else
#include <values.h>
#endif
#include "vc.h"
#include "vcint.h"
#include "vcdbl.h"
#include "vcdecom.h"
#include "vcmap.h"
#include "vcenco.h"
#include "vcxstrm.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcint.cpp 1.49 1998/12/09 05:12:00 dwight Exp $";

char vc_int::buf[100];
static VcIOHackStr intbuf;

vc_int::vc_int() { i = 0; }
//vc_int::vc_int(int64_t i2) { i = i2; }
vc_int::vc_int(const vc_int &v) {  i = v.i; }
vc_int::vc_int(long long i2) {i = i2;}

vc_int::~vc_int() { }

vc_int::operator long() const {
    if(sizeof(long) <= sizeof(i))
		return i;
    if(i > LONG_MAX || i < LONG_MIN)
    {
        USER_BOMB("integer truncation", 0);
    }
    else
        return i;
}
vc_int::operator int () const {
    if(sizeof(int) <= sizeof(i))
		return i;
	if(i > INT_MAX || i < INT_MIN)
	{
		USER_BOMB("long truncation",0);
	}
	else
		return i;
}
vc_int::operator void *() const {
    if(sizeof(void *) > sizeof(i))
	{
        USER_BOMB("pointer truncation", 0);
	}
	return (void *)i;
}
vc_int::operator double() const {return (double)i; }
vc_int::operator long long() const {
    if(sizeof(long long) <= sizeof(i))
        return i;
    if(i > LLONG_MAX || i < LLONG_MIN)
    {
        USER_BOMB("integer truncation", 0);
    }
    else
        return i;
}
//vc_int::operator int64_t() const {return i; }
vc_int::operator const char *() const {USER_BOMB("can't convert int to string (unimp)", "0");}

const char *
vc_int::peek_str() const
{
    intbuf.reset();
    intbuf << i;
    char z = 0;
    intbuf.append(&z, 1);
    return intbuf.ref_str();
}

void
vc_int::stringrep(VcIO o) const {o << i;}

vc
vc_int::operator+(const vc &v) const {return v.int_add(*this);}
vc
vc_int::operator-(const vc &v) const {return v.int_sub(*this);}
vc
vc_int::operator*(const vc &v) const {return v.int_mul(*this);}
vc
vc_int::operator/(const vc &v) const {return v.int_div(*this);}
vc
vc_int::operator%(const vc &v) const {return v.int_mod(*this);}

#define PROLOG     vc retval;vc_int *v1 = new vc_int((const vc_int &)v)
#define EPILOG   retval.redefine(v1);return retval;
vc
vc_int::int_add(const vc& v) const {PROLOG; v1->i += i;EPILOG}
vc
vc_int::int_sub(const vc& v) const {PROLOG; v1->i -= i;EPILOG}
vc
vc_int::int_mul(const vc& v) const {PROLOG; v1->i *= i;EPILOG}
vc
vc_int::int_div(const vc& v) const {PROLOG; v1->i /= i;EPILOG}
vc
vc_int::int_mod(const vc& v) const {PROLOG; v1->i %= i;EPILOG}

#undef PROLOG
#undef EPILOG

int
vc_int::int_lt(const vc& v) const {return ((const vc_int&) v).i < i;}
int
vc_int::int_le(const vc& v) const {return ((const vc_int&) v).i <= i;}
int
vc_int::int_gt(const vc& v) const {return ((const vc_int&) v).i > i;}
int
vc_int::int_ge(const vc& v) const {return ((const vc_int&) v).i >= i;}
int
vc_int::int_eq(const vc& v) const {return ((const vc_int&) v).i == i;}
int
vc_int::int_ne(const vc& v) const {return ((const vc_int&) v).i != i;}

enum vc_type
vc_int::type() const { return VC_INT; }

int
vc_int::operator <(const vc &v) const {  return v.int_lt(*this); }
int
vc_int::operator <=(const vc &v) const {  return v.int_le(*this); }
int
vc_int::operator >(const vc &v) const {  return v.int_gt(*this); }
int
vc_int::operator >=(const vc &v) const {  return v.int_ge(*this); }
int
vc_int::operator ==(const vc &v) const {  return v.int_eq(*this); }
int
vc_int::operator !=(const vc &v) const {  return v.int_ne(*this); }

hashValueType
vc_int::hashValue() const {return (hashValueType)(((i>>16)&0xffff) + (i&0xffff));}

void
vc_int::printOn(VcIO outputStream) { outputStream << i;}

#define PROLOG     vc retval;vc_double *v1 = new vc_double((const vc_double&)v)
#define EPILOG   retval.redefine(v1);return retval
vc vc_int::double_add(const vc& v) const {PROLOG; v1->d += i;EPILOG;}
vc vc_int::double_sub(const vc& v) const {PROLOG; v1->d -= i;EPILOG;}
vc vc_int::double_mul(const vc& v) const {PROLOG; v1->d *= i;EPILOG;}
vc vc_int::double_div(const vc& v) const {PROLOG; v1->d /= i;EPILOG;}
vc vc_int::double_mod(const vc& ) const {USER_BOMB("can't modulo floating numbers", vcnil);}
#undef PROLOG
#undef EPILOG

#define PROLOG     vc retval;vc_vector *v1 = new vc_vector(((const vc_vector&)v).num_elems());
#define EPILOG   retval.redefine(v1);return retval;
vc vc_int::vec_add(const vc& v) const {PROLOG; v1->scal1_add((const vc_vector&)v, *this); EPILOG}
vc vc_int::vec_sub(const vc& v) const {PROLOG; v1->scal1_sub((const vc_vector&)v, *this);EPILOG}
vc vc_int::vec_mul(const vc& v) const {PROLOG; v1->scal1_mul((const vc_vector&)v, *this);EPILOG}
vc vc_int::vec_div(const vc& v) const {PROLOG; v1->scal1_div((const vc_vector&)v, *this);EPILOG}
vc vc_int::vec_mod(const vc& v) const {PROLOG; v1->scal1_mod((const vc_vector&)v, *this);EPILOG}

#undef PROLOG
#undef EPILOG

int vc_int::double_lt(const vc& v) const {return ((const vc_double&)v).d <  i;}
int vc_int::double_le(const vc& v) const {return  ((const vc_double&)v).d <= i;}
int vc_int::double_gt(const vc& v) const {return ((const vc_double&)v).d >  i;}
int vc_int::double_ge(const vc& v) const {return  ((const vc_double&)v).d >= i;}
int vc_int::double_eq(const vc& v) const {return  ((const vc_double&)v).d == i;}
int vc_int::double_ne(const vc& v) const {return  ((const vc_double&)v).d != i;}


long
vc_int::xfer_out(vcxstream& vcx)
{
	char buf[100];
    char buf2[100];

    int bytes = compat_ltoa(i, buf, sizeof(buf));
    if(pltoa(bytes, buf2, sizeof(buf2), 2) != 2)
        oopanic("bad int xfer out");

	char *lp = vcx.out_want(bytes + 2); // +2 for length
	if(lp == 0)
		return -1;
    memcpy(lp, buf2, 2);
	memcpy(lp + 2, buf, bytes);
	return bytes + 2;
}

long
vc_int::xfer_in(vcxstream& vcx)
{
	char *lp = vcx.in_want(2);
	if(lp == 0)
		return EXIN_DEV;
    int len = decode_len(lp);
    if(len == -1 || len == 0)
        return EXIN_PARSE;
    if(len > vcx.max_element_len)
        return EXIN_PARSE;

	lp = vcx.in_want(len);
	if(lp == 0)
		return EXIN_DEV;
    int error;
    i = decode_long2(lp, len, error);
    if(error)
        return EXIN_PARSE;
	return len + 2;
}

