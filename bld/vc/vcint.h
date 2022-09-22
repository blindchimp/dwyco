
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCINT_H
#define VCINT_H
// $Header: g:/dwight/repo/vc/rcs/vcint.h 1.47 1997/10/05 17:27:06 dwight Stable $

#include <stdint.h>
#include "vcatomic.h"
class vcxstream;

//
// Integer atoms. The primary difference between this
// and string atoms is that arithmetic ops are defined
// for these, (they draw errors on strings.)
//
class vc_int : public vc_atomic
{
friend class vc_double;
protected:
	// note: may want to force this to 32bits
	// optionally in 64bit environments, for compat
	// with older interpreters.
    // note: windows 64bit compilations "long" is 32 bit.
    // in a few places we assume we can fit a pointer into a long
    // which breaks ms 64bit builds.

    long long i;
	static char buf[100];

public:
	vc_int() ;
    //vc_int(int64_t i2) ;
	vc_int(const vc_int &v) ;
    vc_int(long long);
	virtual ~vc_int() ;
#ifdef _Windows
        //operator int64_t() const;
#endif
	operator long() const ;
	operator int () const ;
	operator double() const ;
	operator const char *() const ;
	operator void *() const ;
    //operator int64_t() const;
    operator long long() const;

	const char *peek_str() const ;
	void stringrep(VcIO o) const ;
//	//note: this is a hack that shouldn't
//	// be used.
//	void plus(int p) {i += p;}

	virtual vc operator+(const vc &v) const ;
	virtual vc operator-(const vc &v) const ;
	virtual vc operator*(const vc &v) const ;
	virtual vc operator/(const vc &v) const ;
	virtual vc operator%(const vc &v) const ;

	vc int_add(const vc& v) const ;
	vc int_sub(const vc& v) const ;
	vc int_mul(const vc& v) const ;
	vc int_div(const vc& v) const ;
	vc int_mod(const vc& v) const ;


	vc double_add(const vc& v) const;
	vc double_sub(const vc& v) const;
	vc double_mul(const vc& v) const;
	vc double_div(const vc& v) const;
	vc double_mod(const vc& v) const;

	vc vec_add(const vc& v) const;
	vc vec_sub(const vc& v) const;
	vc vec_mul(const vc& v) const;
	vc vec_div(const vc& v) const;
	vc vec_mod(const vc& v) const;

	int int_lt(const vc& v) const ;
	int int_le(const vc& v) const ;
	int int_gt(const vc& v) const ;
	int int_ge(const vc& v) const ;
	int int_eq(const vc& v) const ;
	int int_ne(const vc& v) const ;

	int double_lt(const vc& v) const ;
	int double_le(const vc& v) const ;
	int double_gt(const vc& v) const ;
	int double_ge(const vc& v) const ;
	int double_eq(const vc& v) const ;
	int double_ne(const vc& v) const ;
		
	enum vc_type type() const ;

	virtual int operator <(const vc &v) const ;
	virtual int operator <=(const vc &v) const ;
	virtual int operator >(const vc &v) const ;
	virtual int operator >=(const vc &v) const ;
	virtual int operator ==(const vc &v) const ;
	virtual int operator !=(const vc &v) const ;

	hashValueType hashValue() const ;
	void printOn(VcIO outputStream) ;
#ifndef NO_VCEVAL
    vc translate(VcIO) const;
#endif

	virtual long xfer_out(vcxstream&);
	virtual long xfer_in(vcxstream&);
};

class vc_int_dtor : public vc_int
{
protected:
	vc_int_dtor_fun dt;

public:
	vc_int_dtor(vc_int_dtor_fun d = 0) : vc_int() { dt = d;}
        //vc_int_dtor(int64_t i2, vc_int_dtor_fun d = 0) : vc_int(i2) {dt = d;}
        vc_int_dtor(void *i2, vc_int_dtor_fun d = 0) : vc_int((int64_t)i2) {dt = d;}
        virtual ~vc_int_dtor() {if(dt) (*dt)((void *)i); i = 0;}
	virtual vc_default *do_copy() const {oopanic("copy int_dtor?"); return 0;}
        virtual int close()  {if(dt) (*dt)((void *)i); i = 0; return 0;}
};
#endif
