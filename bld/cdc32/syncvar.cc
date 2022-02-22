
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/syncvar.cc 1.10 1999/01/10 16:09:48 dwight Checkpoint $
#include "syncvar.h"


SyncVar::SyncVar(const char *n, SyncMap *m) :
    name(n), map(m)
{
}

SyncVar::~SyncVar()
{
    map->del(name);
}

int
SyncVar::valid()
{
    return map->contains(name);
}

SyncVar::operator int()
{
    vc v;
    if(map->find(name, v))
        return (int)v;
    return 0;
}

SyncVar::operator bool()
{
    vc v;
    if(map->find(name, v))
        return (int)v;
    return 0;
}

SyncVar::operator vc()
{
    vc v;
    if(map->find(name, v))
        return v;
    return vcnil;
}

//SyncVar::operator const char *()
//{
//// dicey, probably should copy it out or remove this function somehow
//    vc v;
//    if(map->find(name, v))
//        return (const char *)v;
//    return 0;
//}

SyncVar&
SyncVar::operator=(int i)
{
    vc var = vc(i);
    map->replace(name, var);
    return *this;
}

SyncVar&
SyncVar::operator=(bool i)
{
    vc var = vc((int)i);
    map->replace(name, var);
    return *this;
}

SyncVar&
SyncVar::operator=(vc v)
{
    map->replace(name, v);
    return *this;
}

//SyncVar&
//SyncVar::operator=(const char *v)
//{
//    map->replace(name, vc(v));
//    return *this;
//}


SyncManage::SyncManage(SyncMap *m) :
    sendq(vc(VC_VECTOR))
{
    map = m;
    oldmap = new SyncMap(vcnil);
}

SyncManage::~SyncManage()
{
    delete oldmap;
}

void
SyncManage::snapshot()
{
    delete oldmap;
    oldmap = copy();
}

SyncMap *
SyncManage::copy()
{
    SyncMap *v = new SyncMap(vcnil);
    SyncMapIter i(map);
    for(; !i.eol(); i.forward())
    {
        SyncMapAssoc a = i.get();
        v->add(a.peek_key(), a.peek_value());
    }
    return v;
}

vc
SyncManage::diff()
{
    SyncMapIter i(map);
    for(; !i.eol(); i.forward())
    {
        vc val;
        SyncMapAssoc a = i.get();
        if(oldmap->find(a.peek_key(), val))
        {
            if(val != a.peek_value())
            {
                send("r", a);
            }
            oldmap->del(a.peek_key());
        }
        else
        {
            send("a", a);
        }
    }
    // remaining values in old_map are values
    // that have been deleted from the new map
    SyncMapIter j(oldmap);
    for(; !j.eol(); j.forward())
    {
        SyncMapAssoc a = j.get();
        send("d", a.peek_key());
    }
    delete oldmap;
    oldmap = copy();

    vc tmp = sendq;
    sendq = vc(VC_VECTOR);
    return tmp;
}

void
SyncManage::send(const char *what, vc key)
{
    vc v(VC_VECTOR);
    v.append(what);
    v.append(key);
    sendq.append(v);
}

void
SyncManage::send(const char *what, const SyncMapAssoc& a)
{
    vc v(VC_VECTOR);
    v.append(what);
    v.append(a.peek_key());
    v.append(a.peek_value());
    sendq.append(v);
}

#undef TESTSYNC
#ifdef TESTSYNC

#include "iostream.h"

main(int argc, char **argv)
{
    SyncMap m;
    SyncVar& v1 = *new SyncVar("mumble", &m);
    SyncVar& v2 = *new SyncVar("bar", &m);

    SyncManage sm(&m);

    vc diff = sm.diff();
    diff.print_top(cout);
    diff = sm.diff();
    diff.print_top(cout);

    v1 = 3;
    diff = sm.diff();
    diff.print_top(cout);
    diff = sm.diff();
    diff.print_top(cout);

    delete &v1 ;
    v2 = 10;
    diff = sm.diff();
    diff.print_top(cout);
    diff = sm.diff();
    diff.print_top(cout);

    SyncVar v3("baz", &m);
    v3 = 8;
    diff = sm.diff();
    diff.print_top(cout);
    diff = sm.diff();
    diff.print_top(cout);



}

#endif
