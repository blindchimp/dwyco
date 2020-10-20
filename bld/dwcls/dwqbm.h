
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWQBM_H
#define DWQBM_H
// this template allows you to enter a list of objects via add/del, and
// issue simple queries on members of the objects.
// project can be used to create a list containing copies of
// the members from the template class.
// this is useful for situations where you have a bunch of structs
// that have a field (like an identifier), and you want to search for
// the structs that have that identifier.
#include  "dwvec.h"
#include  "dwvecp.h"
#undef index

namespace dwyco
{

template<class T>
class DwQueryByMember
{
public:
    DwQueryByMember() {}

    DwVecP<T> objs;

    void add(T *);
    void del(T *);

    template<typename U> DwVecP<T> query_by_member(const U& val, U T::*memberp);
    template<typename U> int exists_by_member(const U& val, U T::*memberp);
    template<typename U> DwVec<U> project(U T::*memberp);
    int exists_by_fun(int (T::*memfun)(), int val);
    template<typename U, typename V> DwVecP<T> query_by_fun(const U& a1, const V& a2, int (T::*memfun)(const U&, const V&), int val);
    int count() {return objs.num_elems();}

};

template<class T>
void
DwQueryByMember<T>::add(T *a)
{
    if(!objs.contains(a))
        objs.append(a);
}

template<class T>
void
DwQueryByMember<T>::del(T *a)
{
    long i;
    if((i = objs.index(a)) == -1)
        return;
    objs.del(i);
}

template<class T>
template<typename U>
DwVecP<T>
DwQueryByMember<T>::query_by_member(const U& val, U T::* memberp)
{
    int n = objs.num_elems();
    DwVecP<T> ret;
    for(int i = 0; i < n; ++i)
    {
        if(objs[i]->*memberp == val)
            ret.append(objs[i]);
    }
    return ret;
}

template<class T>
template<typename U>
int
DwQueryByMember<T>::exists_by_member(const U& val, U T::* memberp)
{
    int n = objs.num_elems();
    for(int i = 0; i < n; ++i)
    {
        if(objs[i]->*memberp == val)
            return 1;
    }
    return 0;
}

template<class T>
template<typename U>
DwVec<U>
DwQueryByMember<T>::project(U T::* memberp)
{
    int n = objs.num_elems();
    DwVec<U> ret;
    for(int i = 0; i < n; ++i)
    {
        ret.append(objs[i]->*memberp);
    }
    return ret;
}

template<class T>
int
DwQueryByMember<T>::exists_by_fun(int (T::*memfun)(), int val)
{
    int n = objs.num_elems();
    for(int i = 0; i < n; ++i)
    {
        int r = (objs[i]->*memfun)();
        if(r == val)
            return 1;
    }
    return 0;
}

template<class T>
template<typename U, typename V>
DwVecP<T>
DwQueryByMember<T>::query_by_fun(const U& a1, const V& a2, int (T::*memfun)(const U&, const V&), int val)
{
    int n = objs.num_elems();
    DwVecP<T> ret;
    for(int i = 0; i < n; ++i)
    {
        int r = (objs[i]->*memfun)(a1, a2);
        if(r == val)
            ret.append(objs[i]);
    }
    return ret;
}

};

#endif // DWQBM_H
