
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
//
// note: this is intended to be somewhat better performing class
// than its predecessor, DwQueryByMember.
// don't bother using DwQueryByMember2 if you only have 10 or 20 objects
// extant at any given time. this class will be faster for 100 or 1000s
// of objects, but it has significant overhead and some weird restrictions
// (see below) that make it harder to use for anything but really simple classes.
//
// this template allows you to enter object pointers into a list via add/del, and
// issue simple queries on members of the objects.
// this is useful for situations where you have a bunch of structs
// that have a field (like an identifier), and you want to search for
// the structs that have that identifier.
//
// this class never "owns" the objects. it only uses pointers to the
// objects. however, the U and V types
// need operator==. the K type needs operator== and assignment and
// copy ctors if you are using indexing on a member of the T class.
// the "indexer" template class is an internal class, but due to
// vagaries of template expansion of nested subclasses, i couldn't
// isolate it inside DwQueryByMember2 easily.
//
// in your object's constructor, call "add(this)", and in the destructor, call
// "del(this)". those are the only mods you need to the T class to be searched.
//
// NOTE: this indexing assumes you DO NOT CHANGE THE VALUE of the indexed
// struct member (in type T) directly after creating the object. the indexing
// is performed at "add" time, which means the member needs to be set
// in the constructor (in most cases.) likewise, del is done assuming the value
// of the member is intact so it can be unindexed. if you are calling del in a
// destructor, make sure the del happens before you destroy the indexed member.
//
// the member to index is specified as a template argument.
// the member is indexed, and the index is used for searching *on that member*.
// you can still search using other non-indexed members, but the search is linear.
//
// note: there is no destructor for this class right now. it is assumed
// that it will be static most of the time, and short of leak checking this class
// directly, destruction isn't too useful.
//
//
// T is the class of objects you are creating, and want to search. queries return
// vectors of T*
//
// U is the type of the member in T you are searching on. some member in T is declared
// struct T {U mumble;}; NOTE: you do not have to specify the name of the member
// ("mumble", in this case) in the template *declaration*, but you *do* specify it
// in the query function call, eg query_by_member(... &T::mumble...)
//
// K is the type of the member in T that is used for indexing (the "key" type).
// when using indexing, you must name a specific member whose value will be used
// to create the index.
// struct T { U mumble; K key; };
//
// this class figures out if you are querying the "key" member and uses the
// index. otherwise, it uses linear search. the values of the key can have duplicates.

#include "dwvecp.h"
#include "dwtree2.h"
#include "dwbag.h"

#undef index
#define DWQBM_W_IDX(nm, user_class, idx_member) dwyco::DwQueryByMember2<user_class, decltype(user_class::idx_member), &user_class::idx_member> nm
// note: if you don't need an index, but have lots of objects to search, this can
// be a little faster for object creation and deletion (but not for searching.)
#define DWQBM_NO_IDX(nm, user_class) dwyco::DwQueryByMember2<user_class> nm

namespace dwyco
{

template<class UserClass, typename IndexKeyType, IndexKeyType UserClass::* MemberPtr>
struct indexer
{
private:
    indexer& operator=(const indexer&) = delete;
    indexer(const indexer&) = delete;
    // note: no dtor because we don't destruct these things
    ~indexer() = delete;
public:

    typedef DwAssocImp<UserClass *, IndexKeyType> BagAssoc;
    typedef DwBag<BagAssoc> BagIdx;
    typedef dwinternal_pos<BagAssoc> internal_idx_pos;

    BagIdx *b;
    IndexKeyType UserClass::* memberp;

    indexer() {
        b = new BagIdx(BagAssoc(), 101);
        memberp = MemberPtr;
    }

    internal_idx_pos index_obj(UserClass *a) {
        return b->add2(BagAssoc(a, a->*memberp));
    }
    void deindex_obj(internal_idx_pos p) {
        b->del2(p);
    }

    DwVecP<UserClass> find_objs(const IndexKeyType &k) {
        DwVec<BagAssoc> lst = b->find(BagAssoc(0, k));
        int n = lst.num_elems();
        DwVecP<UserClass> ret;
        for(int i = 0; i < n; ++i)
        {
            ret.append(lst[i].get_value());
        }
        return ret;
    }
    int count_objs(const IndexKeyType &k) {
        auto cnt = b->count_keys(BagAssoc(0, k));
        return cnt;

    }
    int exists_objs(const IndexKeyType &k) {
        return b->contains(BagAssoc(0, k));
    }



};

template<class T, class K = int, K T::* MemberPtr = nullptr>
class DwQueryByMember2
{
private:
    DwQueryByMember2& operator=(const DwQueryByMember2&) = delete;
    DwQueryByMember2(const DwQueryByMember2&) = delete;
    typedef dwinternal_pos<DwAssocImp<T *, K> > jesus_fuck_me;

public:

    DwQueryByMember2() {
        if(MemberPtr != nullptr)
            idx = new indexer<T, K, MemberPtr>;
        else
            idx = 0;
        objs = new DwTreeKaz<jesus_fuck_me, T*>(0);
    }
    // note: since these objects are normally static, we don't bother
    // destructing them. if you are doing leak checking, you might
    // want to consider proper deletion here
    ~DwQueryByMember2() {}
private:
    DwTreeKaz<jesus_fuck_me, T*> *objs;
    indexer<T, K, MemberPtr> *idx;

public:

    void add(T *);
    void del(T *);

    template<typename U> DwVecP<T> query_by_member(const U& val, U T::*memberp);
    DwVecP<T> query_by_member(const K& val, K T::*memberp);

    template<typename U> int count_by_member(const U& val, U T::*memberp);
    int count_by_member(const K& val, K T::*memberp);

    template<typename U> int exists_by_member(const U& val, U T::*memberp);
    int exists_by_member(const K& val, K T::*memberp);

    int count() {return objs->num_elems();}

#if 0
    template<typename U> DwVec<U> project(U T::*memberp);
    int exists_by_fun(int (T::*memfun)(), int val);
    template<typename U, typename V> DwVecP<T> query_by_fun(const U& a1, const V& a2, int (T::*memfun)(const U&, const V&), int val);
#endif

};

template<class T, class K, K T::* memp>
void
DwQueryByMember2<T, K, memp>::add(T *a)
{

    // do indexing here
    if(idx && memp == idx->memberp)
    {
        auto pos = idx->index_obj(a);
        objs->add(a, pos);
    }
    else
    {
        objs->add(a, 0);
    }
}

template<class T, class K, K T::* memp>
void
DwQueryByMember2<T, K, memp>::del(T *a)
{
    if(idx && memp == idx->memberp)
    {
        jesus_fuck_me jfm;
        if(!objs->find(a, jfm))
            oopanic("bad del");

        idx->deindex_obj(jfm);
    }
    if(!objs->del(a))
        oopanic("qbm2 del fail");

}

template<class T, class K, K T::* memp>
DwVecP<T>
DwQueryByMember2<T, K, memp>::query_by_member(const K& val, K T::* memberp)
{
    if(idx && memp == memberp)
    {
        auto ret = idx->find_objs(val);
        return ret;
    }
    DwTreeKazIter<jesus_fuck_me, T*> i(objs);
    DwVecP<T> ret;
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            ret.append(p);
    }
    return ret;
}

template<class T, class K, K T::* memp>
template<typename U>
DwVecP<T>
DwQueryByMember2<T, K, memp>::query_by_member(const U& val, U T::* memberp)
{
    DwTreeKazIter<jesus_fuck_me, T*> i(objs);
    DwVecP<T> ret;
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            ret.append(p);
    }
    return ret;
}

template<class T, class K, K T::* memp>
int
DwQueryByMember2<T, K, memp>::count_by_member(const K& val, K T::* memberp)
{
    if(idx && memp == memberp)
    {
        auto cnt = idx->count_objs(val);
        return cnt;
    }
    DwTreeKazIter<jesus_fuck_me, T*> i(objs);
    int cnt = 0;
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            ++cnt;
    }
    return cnt;
}

template<class T, class K, K T::* memp>
template<typename U>
int
DwQueryByMember2<T, K, memp>::count_by_member(const U& val, U T::* memberp)
{
    DwTreeKazIter<jesus_fuck_me, T*> i(objs);
    int cnt = 0;
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            ++cnt;
    }
    return cnt;
}

template<class T, class K, K T::* memp>
int
DwQueryByMember2<T, K, memp>::exists_by_member(const K& val, K T::* memberp)
{
    if(idx && memp == memberp)
    {
        return idx->exists_objs(val);
    }
    DwTreeKazIter<jesus_fuck_me, T*> i(objs);
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            return 1;
    }
    return 0;
}

template<class T, class K, K T::* memp>
template<typename U>
int
DwQueryByMember2<T, K, memp>::exists_by_member(const U& val, U T::* memberp)
{
    DwTreeKazIter<jesus_fuck_me, T*> i(objs);
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().peek_key();
        if(p->*memberp == val)
            return 1;
    }
    return 0;
}





#if 0

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
