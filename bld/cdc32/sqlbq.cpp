
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef _Windows
#ifdef _MSC_VER
#include <direct.h>
#endif
#include <dos.h>
#include <io.h>
#endif

#ifdef DWYCO_USE_STATIC_SQLITE
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "dwrtlog.h"
#include "vc.h"
#include "sqlbq.h"
#include "dwstr.h"

namespace dwyco {
#define USER_BOMB(a, b) {return (b);}

#ifdef DWYCO_DBG_CHECK_SQL

// this is a hack to get around the "unbound arguments are treated as null"
// peculiarity in sqlite. i've been burned directly by this problem several
// times, usually thru typos. this is for debugging only, and
// should be disabled in release. note that it assumes you won't have
// more than ?31 as an arg, and it is broken in cases where you give
// it ? is some other context.
static
void
check_args(const char *sql, int count)
{
    static DwString qargs[32];
    static int been_here;
    if(!been_here)
    {
        for(int i = 0; i < 32; ++i)
        {
            qargs[i] = DwString("?");
            qargs[i] += DwString::fromInt(i + 1);
        }
        been_here = 1;
    }
    unsigned int found = 0;
    DwString a(sql);
    int dense = 0;
    for(int i = 31; i >= 0; --i)
    {
        int gotit = a.srep(qargs[i], "", 1);
        if(gotit)
        {
            dense = 1;
            found |= (1 << i);
        }
        else if(dense)
        {
            oopanic("nondense args to sql");
        }
    }
    int cnt = 0;
    for(int i = 0; i < count; ++i)
    {
        if(!(found & (1 << i)))
        {
            oopanic("unused sql arg");
        }
        found &= ~(1 << i);
        ++cnt;
    }
    if(found)
    {
        oopanic("unspeced sql arg, ?x treated as null");
    }
    if(cnt != count)
    {
        oopanic("#args != speced args");
    }
}
#endif

vc
sqlite3_bulk_query(sqlite3 *dbs, const VCArglist *a)
{
    const VCArglist &aa = *a;
    vc sql = aa.get(0);
    //vc err = aa[2];
    // everything after err is considered to be a binding to be
    // set into the sql
    sqlite3_stmt *st = 0;
    const char *tail = 0;
    int errcode;

#ifdef DWYCO_DBG_CHECK_SQL
    GRTLOG("sql: %d %s", aa.num_elems(), (const char *)sql);
    {
        for(int i = 1; i < aa.num_elems(); ++i)
        {
            GRTLOGVC(aa.get(i));
        }
    }
    check_args(sql, aa.num_elems() - 1);
#endif

    if((errcode = sqlite3_prepare_v2(dbs, sql, sql.len(),
                                     &st, &tail)) != SQLITE_OK)
    {
        throw -1;
        return vcnil;
    }
    if(tail && *tail != 0)
    {
        USER_BOMB("must be exactly 1 sql statement in query", vcnil)
    }
    // bind in vars
    if(a->num_elems() > 1)
    {
        for(int i = 1; i < a->num_elems(); ++i)
        {
            vc val = aa.get(i);
            switch(aa.get(i).type())
            {
            case VC_INT:
                if(sqlite3_bind_int(st, i, val) != SQLITE_OK)
                {
                    sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_STRING:
            case VC_BSTRING:
                if(sqlite3_bind_text(st, i, val, val.len(), SQLITE_TRANSIENT) != SQLITE_OK)
                {
                    sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_DOUBLE:
                if(sqlite3_bind_double(st, i, val) != SQLITE_OK)
                {
                    sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_NIL:
                if(sqlite3_bind_null(st, i) != SQLITE_OK)
                {
                    sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_VECTOR:
                // this is a kluge: we allow specifying a type
                // in the first element, and the value in the second
                // element. this is only used for blobs at the moment,
                // but it could easily be extended to allow this
                // routine to serialize and deserialize stuff for the
                // caller.
                if(val.num_elems() != 2 ||
                        val[0] != vc("blob"))
                {
                    USER_BOMB("sql bind error, explicit type vector must be (type value) pair (and only \"blob\" type is supported now.", vcnil)
                }
                if(sqlite3_bind_blob(st, i, (const void *)(const char *)(val[1]), val[1].len(), SQLITE_TRANSIENT) != SQLITE_OK)
                {
                    sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;

            default:
                USER_BOMB("can't bind that in sql", vcnil)
            }
        }
    }
    vc res(VC_VECTOR);
    int cols = sqlite3_column_count(st);
    while(1)
    {
        int serr = sqlite3_step(st);
        switch(serr)
        {
        case SQLITE_DONE:
            goto out;
        case SQLITE_BUSY:
            // need some indication of what we should do here.
            // retry or abort and error out. maybe do some exception
            // stuff here to allow the user to abort if they want to.
            res = "busy";
            // note: because of bugs in sqlite, it appears that
            // this case requires any transaction to be rolled back
            // to avoid database corruption (wtf, that is pretty sad.)
            // anyway, as long as you are 3.4+ on sqlite, the
            // transaction will be rolled back for you. it doesn't
            // really say how that affects other operations. it is
            // probably best to just issue an explicit rollback sql
            // command just in case.
            break;
        case SQLITE_ROW:
        {
            vc resrow(VC_VECTOR, 0, cols);
            for(int i = 0; i < cols; ++i)
            {
                switch(sqlite3_column_type(st, i))
                {
                case SQLITE_INTEGER:
                    resrow[i] = sqlite3_column_int(st, i);
                    break;
                case SQLITE_FLOAT:
                    resrow[i] = sqlite3_column_double(st, i);
                    break;
                case SQLITE_BLOB:
                    resrow[i] = vc(VC_BSTRING, (const char *)sqlite3_column_blob(st, i), sqlite3_column_bytes(st, i));
                    break;
                case SQLITE_NULL:
                    resrow[i] = vcnil;
                    break;
                case SQLITE_TEXT:
                    resrow[i] = vc(VC_BSTRING, (const char *)sqlite3_column_text(st, i), sqlite3_column_bytes(st, i));
                    break;
                default:
                    oopanic("bogus sqlite type");
                }
            }
            res.append(resrow);
        }
        break;
        default:
            oopanic(sqlite3_errmsg(dbs));
            sqlite3_finalize(st);
            return vcnil;
        }
    }
out:
    ;
    sqlite3_finalize(st);
    return res;
}
}
