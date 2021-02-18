
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCATOMIC_H
#define VCATOMIC_H
// $Header: g:/dwight/repo/vc/rcs/vcatomic.h 1.47 1997/11/27 03:05:59 dwight Stable $

#include "vcdeflt.h"

//
// Base class for atomic vc's. This is where some common
// error functionality is defined. Mostly distinguishes between
// atomics and composites.
//
class vc_atomic : public vc_default
{
protected:
	void bomb_setop() const;
	void bomb_str_rel() const;
	void bomb_call_atom() const;

public:
	vc_atomic();

	virtual int visited();
	virtual void set_visited();

	virtual int is_atomic() const;
	virtual int is_quoted() const;
    virtual int is_map_inhibited() const;

	vc vec_add(const vc& v) const ;
	vc vec_sub(const vc& v) const ;
	vc vec_mul(const vc& v) const ;
	vc vec_div(const vc& v) const ;
	vc vec_mod(const vc& v) const ;
	
	virtual int str_eq(const vc& v) const;
	virtual int str_ne(const vc& v) const;
	virtual int str_le(const vc& v) const;
	virtual int str_lt(const vc& v) const;
	virtual int str_gt(const vc& v) const;
	virtual int str_ge(const vc& v) const;

	virtual int set_eq(const vc& v) const;
	virtual int func_eq(const vc& v) const;

	virtual vc operator()(void) const;
	//virtual vc operator()(void *p) const;
	virtual vc operator()(VCArglist *al) const;

	virtual vc operator()(vc v0) const;
	virtual vc operator()(vc v0, vc v1) const;
	virtual vc operator()(vc v0, vc v1, vc v2) const;
	virtual vc operator()(vc v0, vc v1, vc v2, vc v3) const;

	virtual int num_elems() const;
	virtual int is_empty() const;
	virtual vc& operator[](const vc& v);
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
	virtual vc remove_first2(vc&);
	virtual void remove(const vc& strt, const vc& len);
	virtual void remove(const vc& strt, long len);
	virtual void remove(long strt, const vc& len);
	virtual void remove(long strt, long len);
	virtual void insert(const vc& item, const vc& idx);
	virtual void insert(const vc& item, long idx);

};

#endif
