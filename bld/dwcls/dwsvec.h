#ifndef DWSVEC_H
#define DWSVEC_H
//
// simple vec
// * vector is resized with memcpy
// * vector is dense and append-only
// * copy ctor is called on append
// * dtors of elements are NOT called on destruction of the vector.
// 
#include <string.h>

#define DWSVEC_INITIAL 8
#define DWSVEC_DBG
void oopanic(const char *a);

template<class T>
class DwSVec
{
private:
	DwSVec(const DwSVec&);
	DwSVec& operator=(const DwSVec&);
public:
	char vec[DWSVEC_INITIAL * sizeof(T)];
	char *big;
	int count;
	int real_count;

	inline DwSVec();
	inline ~DwSVec();

	inline void append(const T&);
    inline void append(void *);
	inline const T& ref(int i) const;
	inline const T get(int i) const;
	void set_size(int newsize);

};

template<class T>
inline
DwSVec<T>::DwSVec()
{
	count = 0;
	real_count = DWSVEC_INITIAL;
	big = vec;
}

template<class T>
inline
DwSVec<T>::~DwSVec()
{
	if(big != vec)
		delete [] big;
}
	

template<class T>
inline
void
DwSVec<T>::append(const T& c)
{
#ifdef DWSVEC_DBG
	if(count >= real_count)
		oopanic("bad svec append");
#endif
	new (&((T*)big)[count]) T(c);
	++count;
}

template<class T>
inline
void
DwSVec<T>::append(void *c)
{
#ifdef DWSVEC_DBG
    if(count >= real_count)
        oopanic("bad svec append");
#endif
    ((void **)big)[count] = c;
    ++count;
}

template<class T>
inline
const T&
DwSVec<T>::ref(int i) const
{
#ifdef DWSVEC_DBG
	if(i >= count || i < 0)
		oopanic("bad svec ref");
#endif
	return ((T*)big)[i];
}

template<class T>
inline
const T
DwSVec<T>::get(int i) const
{
#ifdef DWSVEC_DBG
	if(i >= count || i < 0)
		oopanic("bad svec get");
#endif
	return ((T*)big)[i];
}

template<class T>
void
DwSVec<T>::set_size(int newsize)
{
#ifdef DWSVEC_DBG
	if(newsize < count)
		oopanic("bad svec setsize");
#endif
	if(newsize > real_count)
	{
		char *nv = new char[newsize * sizeof(T)];
		memcpy(nv, big, sizeof(T) * count);
		if(big != vec)
		{
			delete [] big;
		}
		big = nv;
		real_count = newsize;
	}
}

#endif
