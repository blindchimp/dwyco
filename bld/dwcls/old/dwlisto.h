
/*
 * $Header: g:/dwight/repo/dwcls/rcs/dwlisto.h 1.11 1997/06/01 04:39:55 dwight Stable095 $
 */
#ifndef DWLISTO_H
#define DWLISTO_H

#include "dwlist.h"

#ifdef USE_INHEAP
#define NEW new(heap)
#else
#define NEW new
#define heap 0
#endif

template<class T>
class DwListO : public DwList<T>
{
private:
	
public:
	DwListO(Allocator *a = Default_alloc);
	DwListO(const DwListO&);
	~DwListO();

//	DwListO& operator=(const DwListO&);

	void insert(T* const &d) {DwList<T>::insert(NEW T(*d));}
	void append(T* const &d) {DwList<T>::append(NEW T(*d));}
	void prepend(T* const &d) {DwList<T>::prepend(NEW T(*d));}
	int remove() { delete get(); return DwList<T>::remove();}
	int remove_first() { delete get_first(); return DwList<T>::remove_first();}
	int remove_last() {delete get_last(); return DwList<T>::remove_last();}
};

template<class T>
DwListO<T>::DwListO(Allocator *a) : DwList<T>(a)
{
}

#if 0
template<class T>
DwListO<T>&
DwListO<T>::operator=(const DwListO<T>& l)
{
	return (DwListO<T>&) DwList<T>::operator=(l);
}

#endif
// note: we can't let this default to using the base constructor
// because we need "do_copy" to call the proper virtual functions
// to get the elements copied. if we don't do this, DwListA::append is
// called, which does the wrong thing (ARM12.7)
template<class T>
DwListO<T>::DwListO(const DwListO<T>& l)
#ifdef USE_INHEAP
	: DwList<T>(l.heap)
#endif
{
	int i;

	do_copy(l);
}

template <class T>
DwListO<T>::~DwListO()
{
	T *data;

	dwlist_foreach(data, *this)
		delete data;
}

template<class T>
class DwListOIter : public DwListIter<T>
{
public:
	DwListOIter(DwListO<T> *t) : DwListIter<T>(t) {}
};

#undef NEW
#undef heap

#endif
