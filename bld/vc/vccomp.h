
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCCOMP_H
#define VCCOMP_H
// $Header: g:/dwight/repo/vc/rcs/vccomp.h 1.47 1997/10/05 17:27:06 dwight Stable $


#include "vcdeflt.h"
#include "dwlista.h"

//
// Base class for all composite vc's (vc's
// that consist of 1 or more sub-vc's)
//
// This is used to guide the quoting facilities
// for evaluation (though this may be hoisted out
// since in some cases it is useful to quote atoms
// too.)
//
// Also used as a hanging place for assorted
// error functionality.
//
// NOTE: this library will never be thread safe during
// traversal of structures without some mods...
class vc_composite : public vc_default
{
private:
        mutable long visit;
	static long Visit_SN;

#if 0
	static DwListA<vc_composite *> Comp_list;
	listelem<vc_composite *> *backptr;
#endif

protected:
	int quoted;
	int dont_map;
	VC_ERR_CALLBACK err_callback;

	vc bomb_funcall() const ;
	virtual void bomb_op() const ;
	virtual void bomb_func(const char *type) const ;
	virtual void bomb_setop() const ;

public:
	vc_composite();
	~vc_composite();

	virtual int visited();
	virtual void set_visited();
	static void new_dfs();

	static void flush_all_cache();

	virtual int is_atomic() const ;
	virtual int is_quoted() const ;
	virtual int is_map_inhibited() const ;
	virtual const char *peek_str() const;
	 
	virtual vc_default *do_copy() const = 0;

	virtual int set_eq(const vc& v) const ;
	virtual int func_eq(const vc& v) const ;
	

	vc operator()(void *p) const ;
	vc operator()(VCArglist *a) const ;

	vc operator()() const ;
	vc operator()(vc v0) const ;
	vc operator()(vc v0, vc v1) const ;
	vc operator()(vc v0, vc v1, vc v2) const ;
	vc operator()(vc v0, vc v1, vc v2, vc v3) const ;

	operator double() const ;
	operator int() const ;
	operator long() const ;
	operator const char *() const ;
	
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

	vc vec_add(const vc& v) const ;
	vc vec_sub(const vc& v) const ;
	vc vec_mul(const vc& v) const ;
	vc vec_div(const vc& v) const ;
	vc vec_mod(const vc& v) const ;

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

	virtual int str_eq(const vc& v) const ;
	virtual int str_ne(const vc& v) const ;
	virtual int str_lt(const vc& v) const ;
	virtual int str_le(const vc& v) const ;
	virtual int str_gt(const vc& v) const ;
	virtual int str_ge(const vc& v) const ;


	virtual int operator <(const vc &v) const ;
	virtual int operator <=(const vc &v) const ;
	virtual int operator >(const vc &v) const ;
	virtual int operator >=(const vc &v) const ;

	virtual vc& operator[](const vc& v) ;
	virtual vc& operator[](int);
	virtual vc& operator[](long);
	virtual const vc& operator[](const vc& v) const;
	virtual const vc& operator[](int) const;
	virtual const vc& operator[](long) const;
	virtual int contains(const vc&);
	virtual int find(const vc&, vc& out);
	virtual void add(const vc&);
	virtual void add_kv(const vc&, const vc&);
	virtual int del(const vc&);
	virtual void append(const vc&);
	virtual vc remove_last();
	virtual void prepend(const vc&);
	virtual vc remove_first();
	virtual vc remove_first2(vc& key_out);
	virtual void remove(const vc& strt, const vc& len);
	virtual void remove(const vc& strt, long len);
	virtual void remove(long strt, const vc& len);
	virtual void remove(long strt, long len);
	virtual void insert(const vc& item, const vc& idx);
	virtual void insert(const vc& item, long idx);
	
	VC_ERR_CALLBACK set_err_callback(VC_ERR_CALLBACK);

};

#endif
