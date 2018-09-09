#ifndef SIMPLESQL_H
#define SIMPLESQL_H

struct sqlite3;
#include "dwrtlog.h"
#include "vc.h"
#include "sqlbq.h"


namespace dwyco {

class SimpleSql
{
    DwString dbname;
    sqlite3 *Db;

public:
    SimpleSql(const DwString& nm) {
        dbname = nm;
        Db = 0;
    }
    ~SimpleSql() {
        if(Db)
            exit();
    }

    void sql_simple(const char *sql);

    virtual void init_schema() = 0;
    void init();
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
