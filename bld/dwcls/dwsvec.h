
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWSVEC_H
#define DWSVEC_H
//
// simple vec
// * vector is resized with memcpy
// * vector is dense and append-only
// * copy/move ctor is called on append
// * dtors of elements are called on destruction of the vector.
// * vector is NOT automatically resized if you append past the end
// * vector has DWSVEC_INITIAL elements before you need to resize it
//   with set_size.
// * WARNING: elements are NOT default-constructed, ever. after vector construction
// or set_size, T's default ctor is NOT used. set_size is used to determine
// storage requirements, and does not affect the "count" of the vector.
// * the only way to increase "count" is to append items.
// * you can decrease count by deleting the first N elements of the vector
// (dtors are called for the removed items)
// * on vector destruction, T's dtor is called for elements 0 to count - 1.
//
#include <string.h>
#include <utility>


#define DWSVEC_INITIAL 8
// this checks basic index arguments for validity.
// it also contains a check for situations where you
// might be holding a reference to the internal vector, and
// panics if you attempt an operation that might move that
// vector someplace else. this produces false positives, but
// is useful for finding that "oops" things were you are
// keeping an address around too long. generally, if you
// are performing modifications to the vector other than
// "append", this probably isn't the right class to use.
#define DWSVEC_DBG
[[noreturn]] void oopanic(const char *a);

template<class T>
class DwSVec
{
private:
    DwSVec(const DwSVec&) = delete;
    DwSVec& operator=(const DwSVec&) = delete;

    char vec[DWSVEC_INITIAL * sizeof(T)];
    char *big;
    int count;
    int real_count;
    inline T& ref(int i);
    inline const T &ref(int i) const;
#ifdef DWSVEC_DBG
    mutable bool did_ref;
#endif
public:
    inline DwSVec();
    inline ~DwSVec();

    inline void append(const T&);
    inline void append(T&&);
    inline void append2(T);
    //inline void append(void *);

    inline T get(int i) const;
    void set_size(int newsize);

    T& operator[](int i) {
        return ref(i);
    }
    const T& operator[](int i) const {
        return ref(i);
    }
    int num_elems() const {
        return count;
    }
    int operator==(const DwSVec& o) const {
        for(int i = 0; i < count; ++i)
            if(!(ref(i) == o.ref(i)))
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
#ifdef DWSVEC_DBG
    did_ref = false;
#endif
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

template<class T>
inline
void
DwSVec<T>::append2(T c)
{
#ifdef DWSVEC_DBG
    if(count >= real_count)
        oopanic("bad svec append");
#endif
    ((T*)big)[count] = c;
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
T &DwSVec<T>::ref(int i)
{
#ifdef DWSVEC_DBG
    if(i >= count || i < 0)
        oopanic("bad svec ref");
    did_ref = true;
#endif
    return ((T*)big)[i];
}

template<class T>
inline
const T &DwSVec<T>::ref(int i) const
{
#ifdef DWSVEC_DBG
    if(i >= count || i < 0)
        oopanic("bad svec ref");
    did_ref = true;
#endif

    return ((const T*)big)[i];
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
    if(newsize < count || did_ref)
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
    if(s != 0 || n > count || n < 0 || did_ref)
        oopanic("bad svec del");
#endif
    for(int i = 0; i < n; ++i)
    {
        (&((T*)big)[i])->~T();
    }
    memmove(big, big + (sizeof(T) * n), (count - n) * sizeof(T));
    count -= n;
}

#endif
