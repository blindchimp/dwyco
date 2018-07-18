
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCNIL_H
#define VCNIL_H
// $Header: g:/dwight/repo/vc/rcs/vcnil.h 1.46 1996/11/17 05:59:00 dwight Stable $

#include "vcatomic.h"
#include "vcxstrm.h"

//
// The nil atom. This is a separate type to avoid the need
// for special values in other classes to represent nil.
// Also, there is an efficiency win here, since all nils
// can use the same storage. The storage is allocated at
// program start time and never freed. The constant
// vcnil is used to reference the nil atom.
//
// Currently, no ops are allowed on nil except for
// relational ops (and even that is a bit suspect.)
// Relational ops only take into account nil-ness of
// other operand.
//
// For the moment, this class is easy about converting
// nil to other types, resulting in the "obvious" value,
// (0, if you convert nil to int, for example.) The values
// returned are in no way "reserved", and users must be
// careful about this.

class vc_nil : public vc_atomic
{
friend class vc;
#ifdef USE_RCT
friend void process_dec();
friend void process_dec(void **);
#endif
private:
	static vc_nil *const vcnilrep;
	void *operator new(size_t bytes) ;

public:
	vc_nil() ;
	virtual ~vc_nil() ;
	operator double() const ;
	operator int() const ;
	operator long() const ;
	operator const char *() const ;

	const char *peek_str() const ;

	vc bomb_nil() const ;
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

	int func_eq(const vc& v) const ;

	enum vc_type type() const ;
	virtual int is_nil() const ;

	virtual int operator <(const vc &v) const ;
	virtual int operator <=(const vc &v) const ;
	virtual int operator >(const vc &v) const ;
	virtual int operator >=(const vc &v) const ;
	virtual int operator ==(const vc &v) const ;
	virtual int operator !=(const vc &v) const ;

	hashValueType hashValue() const ;
	void printOn(VcIO outputStream) ;
	void stringrep(VcIO os) const;
#ifndef NO_VCEVAL
    vc translate(VcIO o) const;
#endif

	virtual long xfer_out(vcxstream&);
	virtual long xfer_in(vcxstream&);
};
#endif
