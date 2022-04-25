#include "netlog.h"
#include <time.h>

using namespace dwyco;

namespace dwyco {

netlog::sqldb *netlog::db;
ssns::signal1<vc> Netlog_signal;

netlog::sqldb::sqldb() :
    SimpleSql("netlog.sql")
{

}

void
netlog::sqldb::init_schema(const DwString&)
{
    sql_simple("create table if not exists netlog( "
" local_ip, local_port, "
" peer_ip, peer_port, "
" peer_uid, "
" time, "
" tube_id, chan_id, "
" event, bytes_out, bytes_in)"
);
    sql_simple("drop view if exists connection_times");

    sql_simple("create view if not exists connection_times(peer_ip, start_tm, secs, uid) as "
"with contime(tube_id, tm) as (select tube_id,max(n1.time) - min(n1.time) from netlog n1,netlog n2 using (tube_id) group by tube_id),"
"pu(uid, tube_id) as (select peer_uid, tube_id from netlog where peer_uid notnull)"
"select peer_ip, min(time), tm, (select uid from pu where tube_id = contime.tube_id)  from contime,netlog using(tube_id) where peer_ip notnull and peer_ip != 'nil' group by tube_id order by min(time)"
);

}

netlog::netlog()
{
    db = 0;
}

void
netlog::start()
{
    db->start_transaction();
}

void
netlog::commit()
{
    db->commit_transaction();
}

void
netlog::rollback()
{
    db->rollback_transaction();
}

int
netlog::init()
{
    if(db)
        return 1;
    db = new sqldb;
    if(!db->init())
    {
        delete db;
        db = 0;
        return 0;
    }
    start();
    db->sql_simple("delete from netlog where strftime('%s', 'now') - time > 3600 * 24 * 3");
    commit();

    return 1;
}

static netlog *Netlog;
#define  sql Netlog->db->simple_sql

int
init_netlog()
{
    if(Netlog)
        return 0;
    Netlog = new netlog;
    if(!Netlog->init())
    {
        delete Netlog;
        Netlog = 0;
        return 0;
    }
    Netlog_signal.connect_ptrfun(netlog::netlog_slot, 1);
    return 1;
}

void
exit_netlog()
{
    if(!Netlog)
        return;
    delete Netlog;
    Netlog = 0;
}

static
DwString
make_columns(int n)
{
    ++n; // for time column
    DwString a("(");
    for(int i = 0; i < n; i += 1)
    {
        a += "?";
        a += DwString::fromInt(i + 1);
        if(i != n - 1)
            a += ",";
    }
    a += ")";
    return a;
}

static
DwString
make_column_names(vc v)
{
    DwString a("(");
    int n = v.num_elems();
    for(int i = 0; i < n; i += 2)
    {
        a += (const char *)v[i];
        //if(i != n - 2)
            a += ",";
    }
    a += "time)";
    return a;
}

void
netlog::netlog_slot(vc v)
{
    if(!Netlog)
        return;
    DwString q("insert into netlog ");
    q += make_column_names(v);
    q += " values ";
    int n = v.num_elems();
    vc vvec(VC_VECTOR);
    for(int i = 1; i < n; i += 2)
    {
        vvec.append(v[i]);
    }
    q += make_columns(vvec.num_elems());
    VCArglist a;
    a.set_size(vvec.num_elems() + 2);
    a.append(q.c_str());
    for(int i = 0; i < vvec.num_elems(); ++i)
        a.append(vvec[i]);
    a.append(time(0));
    try {
        start();
        db->query(&a);
        commit();
    }
    catch (...) {
        rollback();
    }

}
/*
with contime(tube_id, tm) as (select tube_id,max(n1.time) - min(n1.time) from netlog n1,netlog n2 using (tube_id) group by tube_id)
select peer_ip, tm from contime,netlog using(tube_id) where peer_ip notnull and peer_ip != 'nil' group by tube_id order by min(time)

with contime(tube_id, tm) as (select tube_id,max(n1.time) - min(n1.time) from netlog n1,netlog n2 using (tube_id) group by tube_id),
pu(uid, tube_id) as (select peer_uid, tube_id from netlog where peer_uid notnull)
select peer_ip, tm, (select uid from pu where tube_id = contime.tube_id)  from contime,netlog using(tube_id) where peer_ip notnull and peer_ip != 'nil' group by tube_id order by min(time)
*/

}
