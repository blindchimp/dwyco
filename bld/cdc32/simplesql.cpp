#include "simplesql.h"
#ifdef DWYCO_USE_STATIC_SQLITE
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

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
SimpleSql::init(int flags)
{
    if(Db)
        oopanic("already init");
    // note: i did it this way to avoid having to include sqlite.h in simplesql.h
    // this assumes that flags == -1 is invalid for sqlite, which is a stretch, but
    // probably ok. the alternative is to just let the sqlite api leak thru a bit
    // more in the header.
    if(flags == -1)
        flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;
    if(sqlite3_open_v2(newfn(dbnames[0]).c_str(), &Db, flags, 0) != SQLITE_OK)
    {
        Db = 0;
        return 0;
    }
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
SimpleSql::optimize()
{
    try
    {
        sql_simple("pragma optimize");
    }
    catch (...)
    {

    }
}

void
SimpleSql::set_cache_size(int n)
{
    try
    {
        for(int i = 0; i < schema_names.num_elems(); ++i)
        {
            DwString a = DwString("pragma %1.cache_size = -%2").arg(schema_names[i], DwString::fromInt(n));
            sql_simple(a.c_str());
        }
        sql_simple("pragma temp.cache_size = -1000");
    }
    catch (...)
    {

    }
}


void
SimpleSql::attach(const DwString& dbname, const DwString& schema_name)
{
    DwString ndbname;
    if(!dbname.eq(":memory:"))
        ndbname = newfn(dbname);
    else
        ndbname = dbname;
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
        try
        {
            sql_simple("begin  transaction");
        }
        catch(...)
        {
            {
                VCArglist a;
                a.append("rollback transaction");
                sqlite3_bulk_query(Db, &a);
            }
            throw;
        }
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
        // supposed not to throw busy, but it does sometimes. wtf.
        // oh well. the txn is toast anyway, so reduce the depth.
        --tdepth;
        try {
            sql_simple("commit transaction");
        }
        catch(...)
        {
            // we'll do the rollback here, any rollback
            // you have in your caller with also get called.
            {
                VCArglist a;
                a.append("rollback transaction");
                // the docs say the rollback might fail, but it is no bigs.
                // what state the database ends up in? shrug.
                try {
                    sqlite3_bulk_query(Db, &a);
                }
                catch(...)
                {

                }
            }
            // i'm not even sure you want to continue on at this
            // point, if your commits are failing
            check_txn = tmp;
            throw;
        }
    }
    check_txn = tmp;
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
SimpleSql::vacuum()
{
    int tmp = check_txn;
    check_txn = 0;
    try
    {
        for(int i = 0; i < schema_names.num_elems(); ++i)
        {
            DwString a = DwString("vacuum %1").arg(schema_names[i]);
            sql_simple(a.c_str());
        }
    }
    catch (...)
    {

    }
    check_txn = tmp;
}

void
SimpleSql::set_max_size(int mb)
{
   vc pagesize = sql_simple("pragma page_size");
   int sz = pagesize[0][0];

   int pages = (mb * 1024 * 1024) / sz;
   sql_simple(DwString("pragma max_page_count=%1").arg(DwString::fromInt(pages)).c_str());
}

void
SimpleSql::rollback_transaction()
{
    if(tdepth == 0)
    {
        // let's just ignore rollbacks if there is no
        // transaction going. this is useful since
        // starting a transaction can fail (busy)
        // and we put rollbacks in the failure arm
        // of the calling logic. which we need when it is
        // nested, but doesn't really do anything when
        // it is top-level.
        return;
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
#if 0
    if(check_txn && tdepth == 0)
        oopanic("q out of txn");
#endif
    vc res = sqlite3_bulk_query(Db, a);
    if(res.is_nil() || (res.type() == VC_STRING && res == vc("busy")))
        throw -1;
    if(res.type() == VC_STRING && res == vc("full"))
        throw res;
    return res;
}


