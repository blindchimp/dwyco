
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

vc vclh_sha(vc s);
namespace dwyco {


struct Tag_sql : public SimpleSql
{
    Tag_sql() : SimpleSql("tags.sql") {}

    void init_schema() {
        sql_simple("create table if not exists tags ("
                   "uid text collate nocase, "
                   "tag text, "
                   "mid text collate nocase unique on conflict ignore, "
                   "guid text collate nocase unique on conflict ignore, "
                   "time integer "
                   ")");
        sql_simple("create index if not exists guid_idx on tags(guid)");
        sql_simple("create index if not exists time_idx on tags(time)");
        sql_simple("create index if not exists mid_idx on tags(mid)");
        sql_simple("create index if not exists tag_idx on tags(tag)");
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

static
vc
sha_list(vc lst)
{
    DwString str;
    for(int i = 0; i < lst.num_elems(); ++i)
    {
        str += (const char *)lst[i][0];
    }
    vc res = vclh_sha(str.c_str());
    return res;
}

static
vc
flatten(vc lst)
{
    vc res(VC_VECTOR);
    for(int i = 0; i < lst.num_elems(); ++i)
        res.append(lst[i][0]);
    return res;
}

vc
compute_tag_hash(vc tag, vc age_days)
{
    try {
        vc res = Tag_db->sql_simple("select mid from tags where time > strftime('%s', 'now') - ?1 and tag = ?2 order by mid asc",
                                    (long)age_days * 24 * 3600, tag);
        return sha_list(res);
    }
    catch(...)
    {
        return "";
    }
}

vc
ctrl_tag_list(vc tag, vc age_days)
{
    try {
        vc res = Tag_db->sql_simple("select mid from tags where time > strftime('%s', 'now') - ?1 and tag = ?2 order by mid asc",
                                    (long)age_days * 24 * 3600, tag);
        return flatten(res);
    }
    catch(...)
    {
        return vc(VC_VECTOR);
    }
}

}
