
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCDBL_H
#define VCDBL_H
// $Header: g:/dwight/repo/vc/rcs/vcdbl.h 1.47 1997/10/05 17:27:06 dwight Stable $
#include "vcint.h"
class vcxstream;
//
// Floating point atoms
//
class vc_double : public vc_atomic
{
friend class vc_int;
private:
	double d;
	static char buf[2048];
public:
	vc_double() ;
	vc_double(double d2) ;
	vc_double(const vc_int &v) ;
	vc_double(const vc_double &v) ;
	virtual ~vc_double() ;

	operator double() const ;
	operator int() const ;
	operator long() const ;
	operator const char *() const ;

	const char *peek_str() const ;
	void stringrep(VcIO o) const ;

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

	vc vec_add(const vc& v) const;
	vc vec_sub(const vc& v) const;
	vc vec_mul(const vc& v) const;
	vc vec_div(const vc& v) const;
	vc vec_mod(const vc& v) const;

	vc double_add(const vc& v) const ;
	vc double_sub(const vc& v) const ;
	vc double_mul(const vc& v) const ;
	vc double_div(const vc& v) const ;
	vc double_mod(const vc& v) const ;

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

#endif
