
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCSTR_H
#define VCSTR_H
// $Header: g:/dwight/repo/vc/rcs/vcstr.h 1.46 1997/10/05 17:27:06 dwight Stable $

#include "vcatomic.h"
class vcxstream;

//
// String atoms. Note: string atoms are not decomposable, as with
// some "string" packages. One can, however, get at the pointer
// within so you can go back and forth between your favorite
// string package and vc's.
//
class vc_string : public vc_atomic
{
private:
	char *str;
#if (defined(__BCPLUSPLUS__) && __BCPLUSPLUS__ >= 0x500) || defined(__GNUG__) || defined(_MSC_VER)
	mutable int is_cached;
    mutable hashValueType cached_hv;
#else
	int is_cached;
    hashValueType cached_hv;
#endif
    long cached_len;

	void bind(const vc& v) const;
	void local_bind(const vc& v) const;
	void global_bind(const vc& v) const;
	void bremove() const;
	void local_bremove() const;
	void global_bremove() const;
	vc eval() const;
	
    hashValueType hashstr() const;
	void cache_hash() const;

public:
	vc_string(VcNoinit);
	vc_string(const char *s) ;
	vc_string(const vc_string& v) ;
	vc_string(const char *s, int len);
	virtual ~vc_string() ;
	operator const char *() const ; 
	operator int () const ;
	operator double() const ;
	operator long() const ;

	vc_default *do_copy() const;

	const char *peek_str() const ;
	virtual void stringrep(VcIO) const ;

	void bomb(void) const ;
	void bomb_op(void) const ;
	virtual long len() const;
	virtual vc operator+(const vc &v) const ;
	virtual vc operator-(const vc &v) const ;
	virtual vc operator*(const vc &v) const ;
	virtual vc operator/(const vc &v) const ;
	virtual vc operator%(const vc &v) const ;

	enum vc_type type() const ;

	virtual int operator <(const vc &v) const ;
	virtual int operator <=(const vc &v) const ;
	virtual int operator >(const vc &v) const ;
	virtual int operator >=(const vc &v) const ;
	virtual int operator ==(const vc &v) const ;
	virtual int operator !=(const vc &v) const ;

	// in keeping with the convenience hack in vcfuncal.cpp, we
	// allowing calling of strings by mapping first...
	// note: this allows a tight infinite recursion:
	// if you map a a string to itself, and then funcall it,
	// it crashes the system, pronto.
	// 
	virtual vc operator()(void) const ;
	//virtual vc operator()(void *p) const ;
	virtual vc operator()(VCArglist *al) const ;

	virtual vc operator()(vc v0) const ;
	virtual vc operator()(vc v0, vc v1) const ;
	virtual vc operator()(vc v0, vc v1, vc v2) const ;
	virtual vc operator()(vc v0, vc v1, vc v2, vc v3) const ;

	int int_lt(const vc& v) const  ;
	int int_le(const vc& v) const  ;
	int int_gt(const vc& v) const  ;
	int int_ge(const vc& v) const  ;
	int int_eq(const vc& v) const ;
	int int_ne(const vc& v) const ;

	int double_lt(const vc& v) const  ;
	int double_le(const vc& v) const  ;
	int double_gt(const vc& v) const  ;
	int double_ge(const vc& v) const ;
	int double_eq(const vc& v) const ;
	int double_ne(const vc& v) const ;

	virtual int str_eq(const vc& v) const ;
	virtual int str_ne(const vc& v) const ;

	virtual int str_le(const vc& v) const ;
	virtual int str_lt(const vc& v) const ;
	virtual int str_gt(const vc& v) const ;
	virtual int str_ge(const vc& v) const ;

	hashValueType hashValue() const ;
	void printOn(VcIO outputStream) ;

#ifndef NO_VCEVAL
    vc translate(VcIO) const;
#endif

	virtual long xfer_out(vcxstream&);
	virtual long xfer_in(vcxstream&);
};

#endif
