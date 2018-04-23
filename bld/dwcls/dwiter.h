#ifndef DWITER_H
#define DWITER_H
#include <stdlib.h>

template<class T, class I>
class DwIter
{
protected:
	const T *to_iterate;

	DwIter(const T *cont);
	virtual ~DwIter() {}

	virtual void init() = 0;
	virtual void rewind() = 0;
	virtual void fast_forward() = 0;
	virtual void forward() = 0;
	virtual void backward() = 0;
	virtual int eol() = 0;
public:
	virtual I get() ;
};


template<class T, class I>
DwIter<T,I>::DwIter(const T *container)
{
	to_iterate = container;
}

template<class T, class I>
I
DwIter<T,I>::get()
{
	if(eol())
		::abort();
	return to_iterate->get_by_iter(this);
}

#endif

