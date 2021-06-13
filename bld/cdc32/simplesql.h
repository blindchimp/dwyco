#ifndef SIMPLESQL_H
#define SIMPLESQL_H

struct sqlite3;
#include "vc.h"
#include "sqlbq.h"
#include "dwstr.h"


namespace dwyco {

class SimpleSql
{
    DwVec<DwString> dbnames;
    DwVec<DwString> schema_names;
    sqlite3 *Db;

public:
    SimpleSql(const DwString& nm) {
        dbnames[0] = nm;
        schema_names[0] = "main";
        Db = 0;
    }
    virtual ~SimpleSql() {
        if(Db)
            exit();
    }

    // warning: this function doesn't allow you to send nil's
    // into the query.
    vc sql_simple(const char *sql, const vc& = vcnil, const vc& = vcnil, const vc& = vcnil, const vc& = vcnil, const vc& = vcnil);

    virtual void init_schema(const DwString& schema_name) {}
    void attach(const DwString& dbname, const DwString& schema_name);
    void detach(const DwString& schema_name);

    int init();
    void exit();
    void start_transaction();
    void commit_transaction();
    void sync_off();
    void sync_on();
    void rollback_transaction();

    vc query(const VCArglist *a);
};

}

#endif // SIMPLESQL_H
