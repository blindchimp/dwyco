
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWQUERYBYMEMBER_H
#define DWQUERYBYMEMBER_H
#include <QList>


template<class T>
class DwQueryByMember
{
public:
    DwQueryByMember() {}

    QList<T *> objs;

    void add(T *);
    void del(T *);

    template<typename U> QList<T *> query_by_member(const U& val, U T::*memberp);
    template<typename U> QList<U> project(U T::*memberp);

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
    objs.removeOne(a);
}

template<class T>
template<typename U>
QList<T *>
DwQueryByMember<T>::query_by_member(const U& val, U T::* memberp)
{
    int n = objs.count();
    QList<T *> ret;
    for(int i = 0; i < n; ++i)
    {
        if(objs[i]->*memberp == val)
            ret.append(objs[i]);
    }
    return ret;
}

template<class T>
template<typename U>
QList<U>
DwQueryByMember<T>::project(U T::* memberp)
{
    int n = objs.count();
    QList<U> ret;
    for(int i = 0; i < n; ++i)
    {
        ret.append(objs[i]->*memberp);
    }
    return ret;
}

#endif // DWQUERYBYMEMBER_H
