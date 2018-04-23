
/*
 * $Header: g:/dwight/repo/dwcls/rcs/dwveci.h 1.10 1997/06/01 04:40:01 dwight Stable095 $
 */

#ifndef DWVECI_H
#define DWVECI_H

#include "dwvec.h"

template<class T>
class DwVecI : public DwVec<T *>
{
public:
	DwVecI(long init = 0, int is_fixed = 0, int auto_expand = 1) :
    	DwVec<T *>(init, is_fixed, auto_expand) {}
	~DwVecI() {}

	DwVecI(const DwVecI&) {}
	DwVecI& operator=(const DwVecI& v) {
		return (DwVecI&)DwVec<T>::operator=(v);
	}

	// use friend op == (T*, T*) to reuse other funcs in base...
	virtual int operator==(const DwVec<T>&) const;
	virtual int operator!=(const DwVec<T>& t) const {
		return !(*this == t);
	}

	T& operator[](long count) {return *DwVec<T *>::operator[](count);}

	virtual void set_size(long count);
	virtual void append(const T& t) {DwVec<T *>::append(&t);}
	virtual long index(const T& t) const ;
	virtual int contains(const T& t) const ;
	virtual void apply(Funcp f);
};

#endif
