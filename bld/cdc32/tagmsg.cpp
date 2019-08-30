
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <time.h>
#ifdef LINUX
#include <unistd.h>
#endif

#include "vc.h"
#include "xinfo.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "simplesql.h"
#include "tagmsg.h"

using namespace dwyco;

struct Tag_sql : public SimpleSql
{
    Tag_sql() : SimpleSql("tags.sql") {}

    void init_schema() {
        sql_simple("create table if not exists tags ("
                   "uid text collate nocase, "
                   "mid text collate nocase unique on conflict ignore, "
                   "guid text collate nocase unique on conflict ignore, "
                   "time integer "
                   ")");
        sql_simple("create index if not exists guid_idx on tags(guid)");
        sql_simple("create index if not exists time_idx on tags(time)");
        sql_simple("create index if not exists mid on tags(mid)");
    }


};

static Tag_sql *Tag_db;

int
init_tag_sql()
{
    if(Tag_db)
        return 0;
    Tag_db = new Tag_sql;
    return Tag_db->init();
}

void
exit_tag_sql()
{
    if(!Tag_db)
        return;
    Tag_db->exit();
}
