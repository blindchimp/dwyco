#ifndef DWSET_H
#define DWSET_H

#include "dwbag.h"

template <class T> class DwSetIter;

template<class T>
class DwSet : public DwBag<T>
{
friend class DwSetIter<T>;
public:
    DwSet(T def, int t) :
		DwBag<T>(def, t) {}


	void add(const T& key) {
		if(!this->contains(key))
			DwBag<T>::add(key);
	}
	T get_by_iter(DwIter<DwSet<T>, T > *a) const {
		DwSetIter<T> *dbi = (DwSetIter<T> *)a;
		return dbi->list_iter->get();
	} 
};

template<class T>
class DwSetIter : public DwBagIter<T>
{
friend class DwSet<T>;
public:
	DwSetIter(const DwSet<T> *t) : DwBagIter<T>(t) {}

};


#endif
