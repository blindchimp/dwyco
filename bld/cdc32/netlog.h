#ifndef NETLOG_H
#define NETLOG_H
#include "simplesql.h"
#include "ssns.h"

namespace dwyco {
class netlog
{
    struct sqldb : public dwyco::SimpleSql
    {
        sqldb();
        void init_schema(const DwString&);

    };

    static sqldb *db;

public:
    netlog();
    static int init();
    static void netlog_slot(vc);

private:
    static void start();
    static void commit();
    static void rollback();


};
int init_netlog();
void exit_netlog();
extern ssns::signal1<vc> Netlog_signal;

}

#endif // NETLOG_H
