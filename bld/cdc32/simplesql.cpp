#include "simplesql.h"
#ifdef DWYCO_USE_STATIC_SQLITE
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "dwrtlog.h"
#include "vc.h"
#include "sqlbq.h"
#include "fnmod.h"

using namespace dwyco;


vc
SimpleSql::sql_simple(const char *sql, vc a0, vc a1, vc a2)
{
    VCArglist a;
    a.append(sql);
    if(!a0.is_nil())
    {
        a.append(a0);
        if(!a1.is_nil())
        {
            a.append(a1);
            if(!a2.is_nil())
            {
                a.append(a2);
            }
        }
    }
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return res;
}



void
SimpleSql::init()
{
    if(Db)
        oopanic("already init");
    if(sqlite3_open(newfn(dbname).c_str(), &Db) != SQLITE_OK)
    {
        Db = 0;
        return;
    }
    init_schema();
}

void
SimpleSql::exit()
{
    if(!Db)
        return;
    sqlite3_close_v2(Db);
    Db = 0;
}

void
SimpleSql::start_transaction()
{
    sql_simple("savepoint mi;");
}


void
SimpleSql::commit_transaction()
{
    sql_simple("release mi;");
}


void
SimpleSql::sync_off()
{
    sql_simple("pragma synchronous=off;");
}

void
SimpleSql::sync_on()
{
    sql_simple("pragma synchronous=full;");

}

void
SimpleSql::rollback_transaction()
{
    VCArglist a;
    a.append("rollback to savepoint mi;");
    sqlite3_bulk_query(Db, &a);
}

vc
SimpleSql::query(const VCArglist *a)
{
    vc res = sqlite3_bulk_query(Db, a);
    if(res.is_nil())
        throw -1;
    return res;
}


