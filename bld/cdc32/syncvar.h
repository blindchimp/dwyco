
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SYNCVAR_H
#define SYNCVAR_H
// $Header: g:/dwight/repo/cdc32/rcs/syncvar.h 1.2 1999/01/10 16:10:59 dwight Checkpoint $

#include "vc.h"
#include "dwtree2.h"
#include "dwassoc.h"

typedef DwTreeKaz<vc, vc> SyncMap;
typedef DwAssocImp<vc, vc> SyncMapAssoc;
typedef DwTreeKazIter<vc, vc> SyncMapIter;

class SyncVar
{
    SyncVar(const SyncVar&) = delete;
    SyncVar& operator=(const SyncVar&) = delete;

public:
    SyncVar(const char *name, SyncMap *map);
    ~SyncVar();

    SyncVar& operator=(int);
    SyncVar& operator=(bool);
    SyncVar& operator=(vc);
    //SyncVar& operator=(const char *);
    operator int();
    operator bool();
    operator vc();
    //operator const char *();
    int valid();

private:
    SyncMap *map;
    vc name;
};

class SyncManage
{
    SyncManage(const SyncManage&) = delete;
    SyncManage& operator=(const SyncManage&) = delete;

public:
    SyncManage(SyncMap *m);
    ~SyncManage();

    vc diff();
    void snapshot();

private:
    vc sendq;
    SyncMap *oldmap;
    SyncMap *map;

    void send(const char *, vc);
    void send(const char *, const SyncMapAssoc& a);
    SyncMap *copy();

};

#endif
