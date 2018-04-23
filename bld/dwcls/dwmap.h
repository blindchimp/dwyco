
/*
 * $Header: g:/dwight/repo/dwcls/rcs/dwmap.h 1.11 1997/06/01 04:39:56 dwight Stable095 $
 */
#ifndef DWMAP_H
#define DWMAP_H

#include "dwobj.h"
#include "ialloc.h"
#include "dwvec.h"
#include "dwlisto.h"
#include "dwiter.h"
#include "dwassoc.h"

template<class R, class D> class DwMap;

template<class R, class D>
class DwMap
{
friend class DwMapIter<R,D>;
private:
	DwVec<DwMapListO<R,D> > table;
	int table_size;
	int count;
	R def;
	// builtin iterator
	int cur_buck;
	int cur_idx;
	void advance();
	int owns;

public:
	DwMap(R def, int tabsize = 31);
    DwMap(int tabsize = 31);
	DwMap(const DwMap&);
	virtual ~DwMap();

	DwMap& operator=(const DwMap&);

	int num_elems() const {return count;}
	int contains(const D&) ;
    int find(const D&, R& out);
	const R& peek_ref(const D&);
	const R* peek_ptr(const D&);
    R get(const D&);
	R& operator[](const D&);
	int del(const D&);
	void rewind();
	int peek_get(const R*&, const D*&);
	void next();

	DwAssocImp<R,D> get_by_iter(DwIter<DwMap<R,D>, DwAssocImp<R,D> > *a) const {
        DwMapIter<R,D> *dmi = (DwMapIter<R,D> *)a;
		DwAssocImp<R,D> *as = (DwAssocImp<R,D> *)dmi->list_iter->get();
		return *as;
	} 
};


#if 0
template<class R, class D>
class DwAssocO : public DwAssoc<R *,D *>
{
public:

	DwAssocO(R *v, D *k) : DwAssoc<R *,D *>(new R(*v), new D(*k)) {}
	DwAssocO(const R& v, const D& k) : DwAssoc<R *, D*>(new R(v), new D(k)) {}
	~DwAssocO() {delete value; delete key;}
	DwAssocO(const DwAssocO& a) : DwAssoc<R *,D *>(new R(*a.value), new D(*a.key)) {}
	virtual DwAssocO& operator=(const DwAssocO& a) {
		if(this != &a)
		{
			delete value;
            delete key;
			value = new R(*a.value);
			key = new D(*a.key);
		}
		return *this;
	}
	R get_value() const {return *value;}
	D get_key() const {return *key;}
	const R& peek_value() const {return *value;}
	const D& peek_key() const {return *key;}
	R& ref_value() const {return *value;}
};
#endif

template<class R, class D>
class DwMapListO : public DwList<DwAssoc<R,D> >
{

public:
	int sort_fun(DwAssoc<R,D> *const &from_list, DwAssoc<R,D> *const &new_elem);
	int search_fun(DwAssoc<R,D> *const &to_find, DwAssoc<R,D> *const &from_list);

}

template<class D>
u_long
hash(const D& val);


template<class R, class D>
int
DwMapListO<R,D>::sort_fun(DwAssoc<R,D> *const &from_list, DwAssoc<R,D> *const &new_elem)
{
	return new_elem->peek_key() < from_list->peek_key();
}

template<class R, class D>
int
DwMapListO<R,D>::search_fun(DwAssoc<R,D> *const &to_find, DwAssoc<R,D> *const &from_list)
{
	return from_list->peek_key() == to_find->peek_key();
}

template<class R, class D>
DwMap<R,D>::DwMap(R deflt, int tabsz)
	: table(tabsz, 1, 0)
{
	count = 0;
	table_size = tabsz;
	def = deflt;
}

template<class R, class D>
DwMap<R,D>::DwMap(int tabsz)
	: table(tabsz, 1, 0)
{
	count = 0;
	table_size = tabsz;
	def = R();
}

template<class R, class D>
DwMap<R,D>::DwMap(const DwMap<R,D>& m)
	: table(m.table_size, 1, 0)
{
	count = m.count;
	table_size = m.table_size;
	def = m.def;
	cur_buck = m.cur_buck;
	cur_idx = m.cur_idx;
	owns = m.owns;
	table = m.table;
};

template<class R, class D>
DwMap<R,D>&
DwMap<R,D>::operator=(const DwMap<R,D>& m)
{
	count = m.count;
	table_size = m.table_size;
	def = m.def;
	table = m.table;
	cur_buck = m.cur_buck;
	cur_idx = m.cur_idx;
	owns = m.owns;
	return *this; 
};

template<class R, class D>
DwMap<R,D>::~DwMap()
{

}

template<class R, class D>
R
DwMap<R,D>::get(const D& key)
{
	u_long hval = hash(key) % table_size;

	DwAssocOImp<R,D> tmp(def, key);
	DwAssoc<R,D> *result;
	if((result = table[hval].search(&tmp)) == LIST_ERROR)
	{
		return def;
	}
	return result->get_value();
}

template<class R, class D>
int
DwMap<R,D>::find(const D& key, R& out)
{
	u_long hval = hash(key) % table_size;

	DwAssocOImp<R,D> tmp(def, key);
	DwAssoc<R,D> *result;
	if((result = table[hval].search(&tmp)) == LIST_ERROR)
	{
		return 0;
	}
	out = result->get_value();
	return 1;
}

template<class R, class D>
R&
DwMap<R,D>::operator[](const D& key)
{
	u_long hval = hash(key) % table_size;

	DwAssocOImp<R,D> tmp(def, key);
	DwAssoc<R,D> *result;
	if((result = table[hval].search(&tmp)) == LIST_ERROR)
	{
		DwAssocImp<R,D> *toadd = new DwAssocImp<R,D>(def, key);
		table[hval].append(toadd);
		//result = table[hval].get_last(); // was copied in by list
        result = toadd;
		++count;
	}
	return result->ref_value();
}


template<class R, class D>
int
DwMap<R,D>::del(const D& key)
{
	u_long hval = hash(key) % table_size;

	DwAssocOImp<R,D> tmp(def, key);
	DwAssoc<R,D> *result;
	if((result = table[hval].search(&tmp)) == LIST_ERROR)
		return 0;
	delete result;
	table[hval].remove();
	--count;
    return 1;
}

template<class R, class D>
const R*
DwMap<R,D>::peek_ptr(const D& key)
{
	u_long hval = hash(key) % table_size;
	DwAssoc<R,D> *result;
	DwAssocOImp<R,D> tmp(def, key);
	if((result = table[hval].search(&tmp)) == LIST_ERROR)
		return 0;
	return &result->peek_value();
}

template<class R, class D>
const R&
DwMap<R,D>::peek_ref(const D& key)
{
	u_long hval = hash(key) % table_size;
	DwAssoc<R,D> *result;
	DwAssocOImp<R,D> tmp(def, key);
	if((result = table[hval].search(&tmp)) == LIST_ERROR)
		return def;
	return result->peek_value();
}

template<class R, class D>
int
DwMap<R,D>::contains(const D& key)
{
	u_long hval = hash(key) % table_size;

	DwAssocOImp<R,D> tmp(def, key);
	if(table[hval].search(&tmp) == LIST_ERROR)
		return 0;
	return 1;
}

template<class R, class D>
void
DwMap<R,D>::advance()
{
	while(cur_buck < table_size && table[cur_buck].num_elems() == 0)
		++cur_buck;
	if(cur_buck < table_size)
		table[cur_buck].rewind();
}

template<class R, class D>
void
DwMap<R,D>::rewind()
{
	cur_buck = 0;
	cur_idx = 0;
    advance();
}

template<class R, class D>
int
DwMap<R,D>::peek_get(const R*& value, const D*& key)
{
	if(cur_buck >= table_size)
	{
		return 0;
	}
	DwAssoc<R,D> *a = table[cur_buck].get();
	if(a == LIST_ERROR)
    	return 0;
	value = &a->peek_value();
	key = &a->peek_key();
	return 1;
}

template<class R, class D>
void
DwMap<R,D>::next()
{
	++cur_idx;
	if(cur_idx >= table[cur_buck].num_elems())
	{
		cur_idx = 0;
		++cur_buck;
		advance();
		return;
	}
	table[cur_buck].forward();
}

template<class R, class D>
class DwMapIter : public DwIter<DwMap<R,D>, DwAssocImp<R,D> >
{
friend class DwMap<R,D>;
private:
	int cur_buck;
	DwListIter<DwAssoc<R,D> > *list_iter;
	void advance();

public:
	DwMapIter(const DwMap<R,D> *t) : DwIter<DwMap<R,D>, DwAssocImp<R,D> >(t) {
		 cur_buck = 0; list_iter = 0; advance();
	}
    ~DwMapIter() {if(list_iter != 0) delete list_iter;}
	void init() {cur_buck = 0; advance();}
	void rewind() {init();}
	void fast_forward() {::abort();}
	void forward();
	void backward() {::abort();}
	int eol() {return cur_buck >= to_iterate->table_size; }
};

template<class R, class D>
void
DwMapIter<R,D>::advance()
{
	if(list_iter != 0)
    	delete list_iter;
	while(cur_buck < to_iterate->table_size && to_iterate->table[cur_buck].num_elems() == 0)
		++cur_buck;
	if(cur_buck < to_iterate->table_size)
	{
		list_iter = new DwListIter<DwAssoc<R,D> >(&to_iterate->table[cur_buck]);
	}
	else
    	list_iter = 0;
}


template<class R, class D>
void
DwMapIter<R,D>::forward()
{
	if(eol())
    	return;
	list_iter->forward();
	if(list_iter->eol())
	{
		delete list_iter;
        list_iter = 0;
		++cur_buck;
		advance();
		return;
	}
}
#endif
