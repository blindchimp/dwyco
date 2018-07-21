
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCDECOM_H
#define VCDECOM_H
// $Header: g:/dwight/repo/vc/rcs/vcdecom.h 1.48 1997/10/05 17:27:06 dwight Stable $


#include "vc.h"
#include "vccomp.h"
#include "vcio.h"

#include "dwvec.h"
class vcxstream;

#include "dwlista.h"
#include "dwbagr.h"
#include "dwsetr.h"
#include "dwbag.h"
#include "dwset.h"
#include "dwmapr.h"
#include "dwtree2.h"
typedef DwBag<vc> VcBagImp;
typedef DwBagIter<vc> VcBagIter;
typedef DwSet<vc> VcSetImp;
typedef DwSetIter<vc> VcSetIter;
typedef DwMapR<vc,vc> VcMapImp;
typedef DwMapRIter<vc,vc> VcMapIter;
typedef DwTreeKaz<vc,vc> VcTreeImp;
typedef DwTreeKazIter<vc,vc> VcTreeIter;
typedef DwTreeKazIter<vc,vc> VcTreeIterPre;


typedef DwVecIter<vc> VcVecIter;

class vc_decomposable : public vc_composite
{
private:
	static long SN;
	long serial_number;

	virtual int encode_numelems(vcxstream&);
	virtual int decode_numelems(vcxstream&, long&);
	virtual long encode_elems(vcxstream&) = 0;
	virtual long decode_elems(vcxstream&, long) = 0;

public:
	vc_decomposable();
	~vc_decomposable() {}

	void bomb_op() const;

	mutable int iterators;

	virtual int is_quoted() const {return TRUE;}
	virtual int is_decomposable() const {return TRUE;}
    virtual hashValueType hashValue() const {return serial_number;}
	virtual int operator ==(const vc &v) const {return v.set_eq(*this);}
	virtual int operator !=(const vc &v) const {return !v.set_eq(*this);}

	virtual vc operator+(const vc &v) const ;
	virtual vc operator-(const vc &v) const ;
	virtual vc operator*(const vc &v) const ;
	virtual vc operator/(const vc &v) const ;
	virtual vc operator%(const vc &v) const ;

	virtual int set_eq(const vc& v) const {
		return serial_number == ((const vc_decomposable&)v).serial_number;
	}
	
	//virtual VcIO operator<<(VcIO os) {os << d; return os;}

	void printOn(VcIO outputStream) {
		outputStream << "decomposable";
	}
	virtual long xfer_out(vcxstream&);
	virtual long xfer_in(vcxstream&);

};



class vc_list_set : public vc_decomposable
{
private:
	DwListA<vc> list;

	void bomb_rem_list();

public:
	vc_list_set() : list(vcnil) {}
	vc_list_set(const vc_list_set& v) : list(v.list) {}

	enum vc_type type() const { return VC_LIST; }
	vc_default *do_copy() const {return new vc_list_set(*this);}
	void foreach(const vc& v, const vc& expr) const;
	void foreach(const vc& v, VC_FOREACH_CALLBACK) const;
    void foreach(const vc& v, VC_FOREACH_CALLBACK2) const;

	void stringrep(VcIO o) const;

	int is_empty() const {return list.num_elems() == 0;}
    int num_elems() const {return list.num_elems();}

	virtual vc& operator[](const vc& v);
	virtual vc& operator[](int);
	virtual vc& operator[](long);
	virtual int contains(const vc&);
    virtual int find(const vc&, vc& out);
	virtual void add(const vc&);
	virtual int del(const vc&);
	virtual void append(const vc&);
    virtual vc remove_last();
	virtual void prepend(const vc&);
    virtual vc remove_first();
	virtual void remove(const vc& strt, const vc& len);
	virtual void remove(const vc& strt, long len);
	virtual void remove(long strt, const vc& len);
	virtual void remove(long strt, long len);
	virtual void insert(const vc& item, const vc& idx);
	virtual void insert(const vc& item, long idx);

	void printOn(VcIO outputStream) ;
	virtual long encode_elems(vcxstream&);
	virtual long decode_elems(vcxstream&, long);
};

class vc_bag : public vc_decomposable
{
    friend class vc;
protected:
	VcBagImp set;

	virtual void bomb_rem();

public:
	vc_bag(int a) : set(vcnil, a) {}
    vc_bag() : set(vcnil, 31) {}
	vc_bag(const vc_bag& v) : set(v.set) {}

	enum vc_type type() const { return VC_BAG; }
	virtual void stringrep(VcIO) const;

	vc_default *do_copy() const {return new vc_bag(*this);}
	void foreach(const vc& v, const vc& expr) const;
	void foreach(const vc& v, VC_FOREACH_CALLBACK) const;
    void foreach(const vc& v, VC_FOREACH_CALLBACK2) const;

	int is_empty() const {return set.num_elems() == 0;}
    int num_elems() const {return set.num_elems();}

	virtual vc& operator[](const vc& v);
	virtual vc& operator[](int);
	virtual vc& operator[](long);
	virtual int contains(const vc&);
    virtual int find(const vc&, vc& out);
	virtual void add(const vc&);
	virtual int del(const vc&);
	virtual void append(const vc&);
    virtual vc remove_last();
	virtual void prepend(const vc&);
    virtual vc remove_first();
	virtual void remove(const vc& strt, const vc& len);
	virtual void remove(const vc& strt, long len);
	virtual void remove(long strt, const vc& len);
	virtual void remove(long strt, long len);
	virtual void insert(const vc& item, const vc& idx);
	virtual void insert(const vc& item, long idx);

	void printOn(VcIO outputStream) ;

	virtual long encode_elems(vcxstream&);
	virtual long decode_elems(vcxstream&, long);


};

class vc_set : public vc_bag
{
private:

	void bomb_rem();

public:
	// note: default ctors/dtors work fine
	vc_set(int a) : vc_bag(a) {}
	vc_set() {}

	enum vc_type type() const { return VC_SET; }
	virtual void stringrep(VcIO) const;

	vc_default *do_copy() const {return new vc_set(*this);}

	virtual void add(const vc&);
	virtual void append(const vc&);
	virtual void prepend(const vc&);
	void printOn(VcIO outputStream) ;
};

class vc_map : public vc_decomposable
{
    friend class vc;

private:
	VcMapImp map;

	void bomb_rem_map();

public:

	vc_map() : map(vcnil, vcnil) {}
    vc_map(int ts) : map(vcnil, vcnil, ts) {}
	vc_map(const vc_map& v) : map(v.map) {}

	enum vc_type type() const { return VC_MAP; }

	vc_default *do_copy() const {
		return new vc_map(*this);
    }

	int is_empty() const {return map.num_elems() == 0;}
	int num_elems() const {return map.num_elems();}
	void stringrep(VcIO) const;
	void foreach(const vc& v, const vc& expr) const;
	void foreach(const vc& v, VC_FOREACH_CALLBACK) const;
    void foreach(const vc& v, VC_FOREACH_CALLBACK2) const;

	virtual vc& operator[](const vc& v);
	virtual vc& operator[](int);
	virtual vc& operator[](long);
	virtual int contains(const vc&);
    virtual int find(const vc&, vc& out);
	virtual void add(const vc&);
	virtual void add_kv(const vc&, const vc&);
	virtual int del(const vc&);
	virtual void append(const vc&);
    virtual vc remove_last();
	virtual void prepend(const vc&);
    virtual vc remove_first();
	virtual void remove(const vc& strt, const vc& len);
	virtual void remove(const vc& strt, long len);
	virtual void remove(long strt, const vc& len);
	virtual void remove(long strt, long len);
	virtual void insert(const vc& item, const vc& idx);
	virtual void insert(const vc& item, long idx);

	void printOn(VcIO outputStream) ;

	virtual long encode_elems(vcxstream&);
	virtual long decode_elems(vcxstream&, long);

};

class vc_int;
class vc_double;

class vc_vector : public vc_decomposable
{
private:
	DwVec<vc> vec;

public:
	vc_vector() {}
	vc_vector(long init) : vec(init) {}
	vc_vector(const vc_vector& v) : vec(v.vec) {}
	enum vc_type type() const { return VC_VECTOR; }
	vc_default *do_copy() const {
		return new vc_vector(*this);
	}

#define decl(name, op) \
	virtual vc vec_##name(const vc&) const;  \
	virtual vc int_##name(const vc&) const; \
	virtual vc double_##name(const vc&) const; \
	virtual void scal1_##name(const vc_vector&, const vc_int&); \
	virtual void scal1_##name(const vc_vector&, const vc_double&); \
	virtual void scal2_int_##name(const vc&, const vc_vector&); \
	virtual void scal2_double_##name(const vc&, const vc_vector&); \
	virtual vc operator op(const vc&) const;

decl(add, +)
decl(sub, -)
decl(mul, *)
decl(div, /)
decl(mod, %)
#undef decl
	void stringrep(VcIO) const;

	int is_empty() const {return vec.num_elems() == 0;}
	int num_elems() const {return vec.num_elems(); }
	void foreach(const vc& v, const vc& expr) const;
	void foreach(const vc& v, VC_FOREACH_CALLBACK) const;
    void foreach(const vc& v, VC_FOREACH_CALLBACK2) const;
	virtual vc& operator[](const vc& v);
	virtual vc& operator[](int);
	virtual vc& operator[](long);
	virtual const vc& operator[](const vc& v) const;
	virtual const vc& operator[](int) const;
	virtual const vc& operator[](long) const;
	virtual int contains(const vc&);
    virtual int find(const vc&, vc& out);
	virtual void add(const vc&);
	virtual int del(const vc&);
	virtual void append(const vc&);
    virtual vc remove_last();
	virtual void prepend(const vc&);
    virtual vc remove_first();
	virtual void remove(const vc& strt, const vc& len);
	virtual void remove(const vc& strt, long len);
	virtual void remove(long strt, const vc& len);
	virtual void remove(long strt, long len);
	virtual void insert(const vc& item, const vc& idx);
	virtual void insert(const vc& item, long idx);
	
	virtual int member_select(const vc& member, vc& out, int toplev, vc_object *topobj);

	void printOn(VcIO outputStream) ;

	virtual long encode_elems(vcxstream&);
	virtual long decode_elems(vcxstream&, long);
};


class vc_tree : public vc_decomposable
{
private:
	VcTreeImp tree;

	void bomb_rem_tree();

public:

	vc_tree() : tree(vcnil) {}
	vc_tree(const vc_tree& v) : tree(v.tree) {}
	~vc_tree() {}

	enum vc_type type() const { return VC_TREE; }

	vc_default *do_copy() const {
		return new vc_tree(*this);
    }

	int is_empty() const {return tree.num_elems() == 0;}
	int num_elems() const {return tree.num_elems();}
	void stringrep(VcIO) const;
	void foreach(const vc& v, const vc& expr) const;
	void foreach(const vc& v, VC_FOREACH_CALLBACK) const;
    void foreach(const vc& v, VC_FOREACH_CALLBACK2) const;

	virtual vc& operator[](const vc& v);
	virtual vc& operator[](int);
	virtual vc& operator[](long);
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

	void printOn(VcIO outputStream) ;

	virtual long encode_elems(vcxstream&);
	virtual long decode_elems(vcxstream&, long);

};
#endif
