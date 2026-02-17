
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
#include "ta.h"
#include "dwstr.h"

#define USER_BOMB(a, b) {return (b);}


// this is a hack to get around the "unbound arguments are treated as null"
// peculiarity in sqlite. i've been burned directly by this problem several
// times, usually thru typos. this is for debugging only, and
// should be disabled in release. note that it assumes you won't have
// more than ?31 as an arg, and it is broken in cases where you give
// it ? in some other context.
namespace dwyco {
#ifdef DWYCO_DBG_CHECK_SQL
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

static
int
final(sqlite3_stmt *s)
{
    int ret = sqlite3_reset(s);
    sqlite3_clear_bindings(s);
    return ret;
}

// perform sqlite3 query prep and execution
//
// the arglist should have
// a[0] as the string sql to be prepared
// a[1..n] are the bindings to be set into the statement where ?1, ?2, etc.
// THE DEBUGGING STUFF ASSUMES YOU ARE USING THE ?n form for parameters.
// if you use other forms (like just '?', or whatever other forms sqlite uses)
// it will confuse things, so just stick with the ?n form.
//
// if stmt_in_out is 0, this just performs the prepare, binds, runs the query
// and finalizes the statement.
//
// if stmt_in_out != 0, and *stmt_in_out == 0
//  the prepare is performed, it is binded, and run, HOWEVER, the statement
//  is RESET and params UNBOUND, and *stmt_in_out is set to the prepared
//  statement that you can cache. it is up to the caller to finalize it.
//
// if *stmt_in_out != 0
//  it is assumed to be an sqlite3_stmt that has already prepared, and is in the
//  state where it has been RESET and can be BOUND to parameters and run. when the
//  query is done, the statement is RESET and UNBOUND.
//
// NOTE: you MUST only provide prepared statements that correspond to the
// same DB connection you provide when the statemnet is prepared.
// random stuff will occur otherwise.
//
// in all cases, the results are returned as a VC_VECTOR, one for each record
// in the result. each record is again a VC_VECTOR.
//
// if there is a generic error, vcnil is returned. this is usually the result of a programming error.
// if there is a SQLITE3_BUSY error, the string "busy" is returned.
// if there is a database full error, the string "full" is returned.
// in these cases, it is up to the caller to determine how to proceed.
//

vc
sqlite3_bulk_query(sqlite3 *dbs, const VCArglist *a, sqlite3_stmt **stmt_in_out)
{
    const VCArglist &aa = *a;
    const vc& sql = aa.get(0);
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

    int (*finalize)(sqlite3_stmt *) = 0;

    if(stmt_in_out != 0 && *stmt_in_out != 0)
    {
        st = *stmt_in_out;
        finalize = final;

    }
    else
    {
        if((errcode = sqlite3_prepare_v2(dbs, sql, sql.len(),
                                         &st, &tail)) != SQLITE_OK)
        {
#ifdef DWYCO_DBG_CHECK_SQL
            oopanic(sqlite3_errmsg(dbs));
            throw -1;
#endif
            TRACK_ADD_str(sqlite3_errmsg(dbs), 1);
            return vcnil;
        }
        if(tail && *tail != 0)
        {
            USER_BOMB("must be exactly 1 sql statement in query", vcnil)
        }
        if(stmt_in_out)
        {
            *stmt_in_out = st;
            finalize = final;
        }
        else
        {
            finalize = sqlite3_finalize;
        }
    }
    // bind in vars
    if(a->num_elems() > 1)
    {
        for(int i = 1; i < a->num_elems(); ++i)
        {
            const vc& val = aa.get(i);
            switch(val.type())
            {
            case VC_INT:
                if(sqlite3_bind_int64(st, i, val) != SQLITE_OK)
                {
                    (*finalize)(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_STRING:
            case VC_BSTRING:
                if(sqlite3_bind_text(st, i, val, val.len(), SQLITE_TRANSIENT) != SQLITE_OK)
                {
                    (*finalize)(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_DOUBLE:
                if(sqlite3_bind_double(st, i, val) != SQLITE_OK)
                {
                    (*finalize)(st);
                    USER_BOMB("sql bind error", vcnil)
                }
                break;
            case VC_NIL:
                if(sqlite3_bind_null(st, i) != SQLITE_OK)
                {
                    (*finalize)(st);
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
                    (*finalize)(st);
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
        case SQLITE_IOERR:
            // need some indication of what we should do here.
            // retry or abort and error out. maybe do some exception
            // stuff here to allow the user to abort if they want to.
            // the IOERR thing seems to be returned on some platforms
            // like android for locking issues.
            res = "busy";
            // it is probably best to just issue an explicit rollback sql
            // command just in case.
            goto out;

            // note: this one is mostly for situations where we are
            // limiting the size of the resulting database for backups
            // or something like that. best thing to do is abort, then
            // vacuum and retry, or maybe adjust the info you are trying
            // to backup. this is mostly for android, where there is a limit
            // on the automatic backup of about 25MB.
        case SQLITE_FULL:
            res = "full";
            goto out;

        case SQLITE_ROW:
        {
            vc resrow(VC_VECTOR, 0, cols);
            for(int i = 0; i < cols; ++i)
            {
                switch(sqlite3_column_type(st, i))
                {
                case SQLITE_INTEGER:
                    resrow[i] = sqlite3_column_int64(st, i);
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
            // note: the volatile here is for debugging, hoping
            // the compiler won't elide it before we can inspect it
            // in the debugger.
            const char *volatile a = sqlite3_errmsg(dbs);
            TRACK_ADD_str(a, 1);
            (*finalize)(st);
            return vcnil;
        }
    }
out:
    ;
    (*finalize)(st);
    GRTLOGVC(res);
    return res;
}
}
