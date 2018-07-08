#ifndef DWSVEC_H
#define DWSVEC_H
//
// simple vec
// * vector is resized with memcpy
// * vector is dense and append-only
// * copy ctor is called on append
// * dtors of elements are called on destruction of the vector.
// * vector is NOT automatically resized if you append past the end
// * vector has DWSVEC_INITIAL elements before you need to resize it
//   with set_size.
//
#include <string.h>
#include <utility>


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
    inline void append(T&&);
    //inline void append(void *);
    inline const T& ref(int i) const;
    inline T get(int i) const;
    void set_size(int newsize);

    const T operator[](int i) const {
        return get(i);
    }
    int num_elems() const {
        return count;
    }
    int operator==(const DwSVec& o) {
        for(int i = 0; i < count; ++i)
            if(!((*this)[i] == o[i]))
                return 0;
        return 1;
    }

    void del(int s, int n = 1);

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
    for(int i = 0; i < count; ++i)
    {
        (&((T*)big)[i])->~T();
    }
    if(big != vec)
        delete [] big;
}


template<class T>
inline
void
DwSVec<T>::append(T&& c)
{
#ifdef DWSVEC_DBG
    if(count >= real_count)
        oopanic("bad svec append");
#endif
    new (&((T*)big)[count]) T(std::move(c));
    ++count;
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


#if 0
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
#endif

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
T
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

template<class T>
void
DwSVec<T>::del(int s, int n)
{
#ifdef DWSVEC_DBG
    if(s != 0 || n > count)
        oopanic("bad svec del");
#endif
    memmove(big, big + (sizeof(T) * n), (count - n) * sizeof(T));
    count -= n;
}

#endif
