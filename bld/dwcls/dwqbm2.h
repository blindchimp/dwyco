
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWQBM2_H
#define DWQBM2_H
// this template allows you to enter a list of objects via add/del, and
// issue simple queries on members of the objects.
// this is useful for situations where you have a bunch of structs
// that have a field (like an identifier), and you want to search for
// the structs that have that identifier.
//
// "project" can be used to create a list containing copies of
// the members from the template class.
//
// this class never "owns" the objects. it only uses pointers to the
// objects. however, the U and V types need to be well behaved:
// they need operator==, assignment.
//
// in your object's constructor, call "add(this)", and in the destructor, call
// "del(this)".
//
// note: searches are linear, so this probably won't work too well if you have
// 100's of objects extant. it wouldn't be hard to introduce indexing
// and stuff, but for current use case, linear is fine.

#include "dwvecp.h"
#include "dwtree2.h"
#include "dwbag.h"

#undef index

namespace dwyco
{

template<class UserClass, typename IndexKeyType, typename UserClass::MemberPtr>
struct indexer
{
    typedef IndexKeyType UserClass::* mumble;

    typedef DwAssocImp<UserClass *, IndexKeyType> BagAssoc;
    typedef DwBag<BagAssoc> BagIdx;

    BagIdx *b;
    IndexKeyType UserClass::* memberp;

    indexer() {
        b = new BagIdx(BagAssoc(), 101);
        memberp = UserClass::MemberPtr;
    }

    void index_obj(UserClass *a) {
        b->add(BagAssoc(a, a->*memberp));
    }

    DwVecP<UserClass> find_objs(IndexKeyType k) {
        DwVec<BagAssoc> lst = b->find(BagAssoc(0, k));
        int n = lst.num_elems();
        DwVecP<UserClass> ret;
        for(int i = 0; i < n; ++i)
        {
            ret.append(lst[i].get_value());
        }
        return ret;
    }



};

template<class T, class I>
class DwQueryByMember2
{
public:
    DwQueryByMember2() : objs(0) {
    }

    DwTreeKaz<int, T*> objs;

    void add(T *);
    void del(T *);

    template<typename U> DwVecP<T> query_by_member(const U& val, U T::*memberp);
#if 0
    template<typename U> int exists_by_member(const U& val, U T::*memberp);
    template<typename U> int count_by_member(const U& val, U T::*memberp);
    template<typename U> DwVec<U> project(U T::*memberp);
    int exists_by_fun(int (T::*memfun)(), int val);
    template<typename U, typename V> DwVecP<T> query_by_fun(const U& a1, const V& a2, int (T::*memfun)(const U&, const V&), int val);
    int count() {return objs.num_elems();}
#endif

};

template<class T, class I>
void
DwQueryByMember2<T, I>::add(T *a)
{
    objs.add(a, 0);
    // do indexing here
}

template<class T, class I>
void
DwQueryByMember2<T, I>::del(T *a)
{
    objs.del(a);
}

template<class T, class I>
template<typename U>
DwVecP<T>
DwQueryByMember2<T, I>::query_by_member(const U& val, U T::* memberp)
{
    DwTreeKazIter<int, T*> i(&objs);
    DwVecP<T> ret;
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            ret.append(p);
    }
    return ret;
}

#if 0

template<class T>
template<typename U>
int
DwQueryByMember2<T>::count_by_member(const U& val, U T::* memberp)
{
    int n = objs.num_elems();
    int ret = 0;
    for(int i = 0; i < n; ++i)
    {
        if(objs[i]->*memberp == val)
            ++ret;
    }
    return ret;
}

template<class T>
template<typename U>
int
DwQueryByMember2<T>::exists_by_member(const U& val, U T::* memberp)
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
DwQueryByMember2<T>::project(U T::* memberp)
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
DwQueryByMember2<T>::exists_by_fun(int (T::*memfun)(), int val)
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
DwQueryByMember2<T>::query_by_fun(const U& a1, const V& a2, int (T::*memfun)(const U&, const V&), int val)
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
#endif

};

#endif // DWQBM_H
