
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef NETLOG_H
#define NETLOG_H
#include "simplesql.h"
#include "ssns.h"

namespace dwyco {
class netlog
{
    struct sqldb : public dwyco::SimpleSql
    {
        sqldb();
        void init_schema(const DwString&);

    };

    static sqldb *db;

public:
    netlog();
    static int init();
    static void netlog_slot(vc);

private:
    static void start();
    static void commit();
    static void rollback();


};
int init_netlog();
void exit_netlog();
extern ssns::signal1<vc> Netlog_signal;

}

#endif // NETLOG_H
