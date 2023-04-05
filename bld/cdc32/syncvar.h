
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


class SyncManage
{
    friend class SyncVar;
    friend class MMChannel;

    SyncManage(const SyncManage&) = delete;
    SyncManage& operator=(const SyncManage&) = delete;

public:
    SyncManage();
    ~SyncManage();

    vc diff();
    void snapshot();
    bool update_available();

private:
    vc sendq;
    SyncMap *oldmap;
    SyncMap *map;
    bool m_update_available;

    void send(const char *, vc);
    void send(const char *, const SyncMapAssoc& a);
    SyncMap *copy();

};

class SyncVar
{
    SyncVar(const SyncVar&) = delete;
    SyncVar& operator=(const SyncVar&) = delete;

public:
    SyncVar(const char *name, SyncManage *map);
    ~SyncVar();

    SyncVar& operator=(int);
    SyncVar& operator=(vc);

    operator int();
    operator vc();

    int valid();

private:
    SyncManage *manager;
    vc name;
};


#endif
