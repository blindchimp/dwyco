
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
init_ctrl_tag_sql()
{
    if(Tag_db)
        return 0;
    Tag_db = new Tag_sql;
    return Tag_db->init();
}

void
exit_ctrl_tag_sql()
{
    if(!Tag_db)
        return;
    Tag_db->exit();
}

static
void
insert_record(vc mid, vc tag)
{
    Tag_db->sql_simple("replace into tags (mid, tag, time, guid) values(?1, ?2, strftime('%s', 'now'), ?3)",
                       mid, tag, to_hex(gen_id()));
}

int
add_ctrl_tag(vc mid, vc tag)
{
    try
    {
        Tag_db->start_transaction();
        insert_record(mid, tag);
        Tag_db->commit_transaction();
        return 1;
    }
    catch (...)
    {
        Tag_db->rollback_transaction();
    }
    return 0;
}
