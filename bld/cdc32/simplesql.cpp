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
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return res;
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
    sql_simple("attach ?1 as ?2", ndbname.c_str(), schema_name.c_str());
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
    sql_simple("detach ?1", schema_name.c_str());
    dbnames.del(i);
    schema_names.del(i);
}

void
SimpleSql::start_transaction()
{
    sql_simple("savepoint ss");
}


void
SimpleSql::commit_transaction()
{
    sql_simple("release ss");
}


void
SimpleSql::sync_off()
{
    sql_simple("pragma synchronous=off;");
}

void
SimpleSql::sync_on()
{
    sql_simple("pragma synchronous=normal;");

}

void
SimpleSql::rollback_transaction()
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

vc
SimpleSql::query(const VCArglist *a)
{
    vc res = sqlite3_bulk_query(Db, a);
    if(res.is_nil())
        throw -1;
    return res;
}


