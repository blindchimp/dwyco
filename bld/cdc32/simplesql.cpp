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
SimpleSql::sql_simple(const char *sql, const vc& a0, const vc& a1, const vc& a2, const vc& a3, const vc& a4)
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
                if(!a3.is_nil())
                {
                    a.append(a3);
                    if(!a4.is_nil())
                    {
                        a.append(a4);
                    }
                }
            }
        }
    }
    return query(&a);
}


void
SimpleSql::set_busy_timeout(int ms)
{
    sqlite3_busy_timeout(Db, ms);
}

int
SimpleSql::init()
{
    if(Db)
        oopanic("already init");
    if(sqlite3_open(newfn(dbnames[0]).c_str(), &Db) != SQLITE_OK)
    {
        Db = 0;
        return 0;
    }
    //sqlite3_busy_timeout(Db, 1000);
    sync_off();
    init_schema(schema_names[0]);
    return 1;
}

void
SimpleSql::exit()
{
    if(!Db)
        return;
    sqlite3_close_v2(Db);
    Db = 0;
    dbnames.del(1, dbnames.num_elems() - 1);
    schema_names.del(1, schema_names.num_elems() - 1);
}

void
SimpleSql::attach(const DwString& dbname, const DwString& schema_name)
{
    DwString ndbname = newfn(dbname);
    if(dbnames.contains(ndbname) || schema_names.contains(schema_name))
        return;
    int tmp = check_txn;
    check_txn = 0;
    sql_simple("attach ?1 as ?2", ndbname.c_str(), schema_name.c_str());
    check_txn = tmp;
    dbnames.append(ndbname);
    schema_names.append(schema_name);
    init_schema(schema_name);
}

void
SimpleSql::detach(const DwString& schema_name)
{
    int i;
    if((i = schema_names.index(schema_name)) == -1)
        return;
    int tmp = check_txn;
    check_txn = 0;
    sql_simple("detach ?1", schema_name.c_str());
    check_txn = tmp;
    dbnames.del(i);
    schema_names.del(i);
}

// note: the monkey business with tdepth is because the "savepoint"
// api doesn't have an immediate mode. also, it seems like
// "release" and "commit" when the savepoint stack becomes empty isn't exactly
// the same. if you do the "release", it destroys the savepoint, but the
// subsequent internally triggered commit can cause a "busy" to be returned.
// then, when you go to "rollback to" your savepoint (whose commit just ostensibly failed),
// the savepoint name is no longer there, and you get a "no such savepoint" error
// from sqlite. it is unclear what state the database is in after this
// sequence of events happens (ie, did the release "fail"? or the "commit"
// or both, and did the rollback happen? maybe get_autocommit would help, but
// even that is unclear from the docs)

void
SimpleSql::start_transaction()
{
    int tmp = check_txn;
    check_txn = 0;
    if(tdepth == 0)
    {
        sql_simple("begin immediate transaction");
    }
    else
    {
        sql_simple("savepoint ss");
    }
    ++tdepth;
    check_txn = tmp;
}


void
SimpleSql::commit_transaction()
{
    if(tdepth == 0)
    {
        oopanic("sqlsimple transaction");
    }
    try {
        int tmp = check_txn;
        check_txn = 0;
    if(tdepth > 1)
    {
        sql_simple("release ss");
        --tdepth;
    }
    else
    {
        // note: if we are using immediate transactions, this is
        // supposed not to throw busy, so we'll just assume if this
        // completes everything is cool, and otherwise the tdepth
        // will be screwed up. if this fails for some reason, the program
        // is on the ropes, and probably needs to exit quickly
        --tdepth;
        sql_simple("commit transaction");
    }
    check_txn = tmp;
    }
    catch(...)
    {
        oopanic("fuck not again");
    }
}


void
SimpleSql::sync_off()
{
    int tmp = check_txn;
    check_txn = 0;
    sql_simple("pragma synchronous=off;");
    check_txn = tmp;
}

void
SimpleSql::sync_on()
{
    int tmp = check_txn;
    check_txn = 0;
    sql_simple("pragma synchronous=normal;");
    check_txn = tmp;
}

void
SimpleSql::rollback_transaction()
{
    if(tdepth == 0)
    {
        oopanic("sqlsimple transaction");
    }
    try {
    if(tdepth > 1)
    {
        {
            VCArglist a;
            a.append("rollback to ss");
            sqlite3_bulk_query(Db, &a);
        }
        {
            VCArglist a;
            a.append("release ss");
            sqlite3_bulk_query(Db, &a);
        }
    }
    else
    {
        {
        VCArglist a;
        a.append("rollback transaction");
        sqlite3_bulk_query(Db, &a);
        }
    }
    }
    catch(...)
    {
        oopanic("rollback error");
    }

    --tdepth;
}

vc
SimpleSql::query(const VCArglist *a)
{
    if(check_txn && tdepth == 0)
        oopanic("q out of txn");
    vc res = sqlite3_bulk_query(Db, a);
    if(res.is_nil() || (res.type() == VC_STRING && res == vc("busy")))
        throw -1;
    return res;
}


