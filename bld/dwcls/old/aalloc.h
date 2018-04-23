
/*
 * $Header: g:/dwight/repo/dwcls/rcs/aalloc.h 1.10 1997/06/01 04:39:32 dwight Stable095 $
 */
#ifndef AALLOC_H
#define AALLOC_H

#define DEFAULT_NHANDLES 100

template<class T> class ALLRET_alloc
{
// huh, this doesn't seem to work
//friend class T;
public:
	ALLRET_alloc(int nhandles = DEFAULT_NHANDLES);
	~ALLRET_alloc();

	T **handle_stack;
	T *handle_space;
	int sp;
	int count;

	T *alloc();
	void free(T *);
};

template<class T>
ALLRET_alloc<T>::ALLRET_alloc(int nhandles)
{
	handle_stack = new T*[nhandles];
	handle_space = new T[nhandles];
	count = nhandles;
	sp = count;
	for(int i = 0; i < count; ++i)
    	handle_stack[i] = &handle_space[i];
}

template<class T>
ALLRET_alloc<T>::~ALLRET_alloc()
{
	delete[] handle_stack;
	delete[] handle_space;
}

template<class T>
T *
ALLRET_alloc<T>::alloc()
{
	if(--sp < 0)
		abort();
	return handle_stack[sp];
}

template<class T>
void
ALLRET_alloc<T>::free(T *h)
{
	if(sp >= count)
		abort();
	handle_stack[sp++] = h;
}



#endif
