#ifndef SIMPLESQL_H
#define SIMPLESQL_H

struct sqlite3;
#include "vc.h"
#include "sqlbq.h"
#include "dwstr.h"


namespace dwyco {

// note: this base class provides NO schema initialization by default.
// this is useful if you  know the database already has some schema.
class SimpleSql
{

private:
    DwVec<DwString> dbnames;
    DwVec<DwString> schema_names;
    sqlite3 *Db;
    int tdepth;

public:
    SimpleSql(const DwString& nm) {
        dbnames[0] = nm;
        schema_names[0] = "main";
        Db = 0;
        tdepth = 0;
        check_txn = 0;
    }
    virtual ~SimpleSql() {
        if(Db)
            exit();
    }
    int check_txn;

    // warning: this function doesn't allow you to send nil's
    // into the query.
    vc sql_simple(const char *sql, const vc& = vcnil, const vc& = vcnil, const vc& = vcnil, const vc& = vcnil, const vc& = vcnil);

    virtual void init_schema(const DwString& schema_name) {}
    void attach(const DwString& dbname, const DwString& schema_name);
    void detach(const DwString& schema_name);

    int init(int flags = -1);
    void exit();
    void optimize();
    void set_cache_size(int);
    void start_transaction();
    void commit_transaction();
    void sync_off();
    void sync_on();
    void vacuum();
    void set_max_size(int mb);
    void rollback_transaction();
    void set_busy_timeout(int ms);

    vc query(const VCArglist *a);
};

}

#endif // SIMPLESQL_H
