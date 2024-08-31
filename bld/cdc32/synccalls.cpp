#include "dlli.h"
#include "ezset.h"
#include "mmchan.h"
#include "dhgsetup.h"
#include "mmcall.h"
#include "pulls.h"
#include "qauth.h"
#include "synccalls.h"
#include "qmsgsql.h"
#include "dirth.h"
#include "cdcpal.h"
#ifdef _Windows
#include <time.h>
#endif

// this is where we figure out what connections to initiate to what other
// devices in our device group.
//
// these are mostly heuristics, trying to balance the frequency we
// attempt connections with the amount of wasted resources caused by
// to much re-trying.
//
// note that we try to connect to *all* other group members, provided we can
// find their ip address. it might make sense to limit this to some subset
// eventually in order to avoid storms of updates. right now, it doesn't
// seem to be a problem given most device groups are fairly small, and most of
// devices are "offline" or unreachable most of the time.
//
// we don't try to figure out the quality of the connections (ie, a direct local
// ip might be better than going through a proxy.) doing something useful in
// this area would require some changes in other parts of the system that aren't
// really worth it at this point (like assessing an incoming connection before
// starting the protocol state machine, and replacing an existing connection.)
//
// there is also a function for creating a data model that can be used to print
// the current state of the set of connections. this isn't too useful for
// users, but can be useful for diagnosing problems while debugging.
//
// note: i tried a klugey way to try and avoid too much livelock (ie, a device
// comes online, and other devices in the group immediately try to
// connect to us while we are connecting to them, resulting in both the
// connections being rejected.)
//
// assuming the clocks on both devices are about ok at the minute level:
// (1) each device breaks up time in to 5 minute blocks.
//   if it is an odd block, this device can only originate calls to lexicographically larger uid's.
//   otherwise, it can only originate calls to smaller uid's.
//
// since all the uid's in the group use the same function, this results in A originating to B for
// 5 minutes, then B to A for 5 minutes. this works, but has some obvious problems with
// the largest and smallest uid's being mostly "receive only" for 5 minutes at a time.
//
// HUGE NOTE: allowing multiple simultaneous connection between two devices is known to
// cause problems with the "delta" processing. i didn't debug it enough to figure it out.
//
using namespace dwyco;
extern vc Online;
extern DwString dwyco::Schema_version_hack;

vc
uids_to_call()
{
    vc ret = Group_uids;
    if(ret.is_nil())
        return vc(VC_VECTOR);
    ret = ret.copy();
    ret.del(My_UID);
    return ret;
}

// note: there are all kinda weird cases with this...
// whether a channel is established or not, etc.
// for now, we just return anything that might look like it
// might be a sync channel, established or not, and it is
// up to the caller to decide what they want.
ChanList
get_all_sync_chans()
{
    ChanList ret;
    DwVecP<MMCall> mmcl = MMCall::calls_by_type("sync");
    for(int i = 0; i < mmcl.num_elems(); ++i)
    {
        MMCall *mmc = mmcl[i];
        MMChannel *mc = MMChannel::channel_by_id(mmc->chan_id);
        if(mc)
            ret.append(mc);
    }

    ChanList cl = MMChannel::channels_by_call_type("sync");
    for(int i = 0; i < cl.num_elems(); ++i)
    {
        if(ret.index(cl[i]) == -1)
            ret.append(cl[i]);
    }
    return ret;
}

static
void
sync_ip(MMChannel *mc, vc v)
{
    if(!mc->proxy_info.is_nil())
    {
        v[M_PROXY] = vctrue;
        DwString ip = (const char *)mc->proxy_info[0];
        ip += ":";
        ip += mc->proxy_info[1].peek_str();
        v[M_IP] = ip.c_str();
    }
    else
    {
        DwString ip = mc->addrstr;
        if(ip.length() == 0 && mc->tube)
            ip = (const char *)mc->tube->remote_addr_ctrl();
        v[M_IP] = ip.c_str();
    }
}

static
int
originate_calls(vc uid)
{
    return 1;

    time_t now = time(0);
    now /= 60;
    now /= 5;
    if(now & 1)
        return uid > My_UID;
    else
        return uid < My_UID;
}

vc
build_sync_status_model()
{
    vc ret(VC_VECTOR);
    vc uids = uids_to_call();
    GRTLOG("SYNCMOD ",0,0);
    GRTLOGVC(uids);
    DwVec<vc> cuids;
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        vc v(VC_VECTOR);
        v[M_UID] = uids[i];
        v[M_STATUS] = originate_calls(uids[i]) ? "od" : "rd"; // disconnected
        v[M_PULLS_ASSERT] = pulls::count_by_uid(uids[i]);
        v[M_PERCENT_SYNCED] = 0;
        ret.append(v);
        cuids.append(uids[i]);
    }

    DwVecP<MMCall> mmcl = MMCall::calls_by_type("sync");
    for(int i = 0; i < mmcl.num_elems(); ++i)
    {
        MMCall *mmc = mmcl[i];
        MMChannel *mc = MMChannel::channel_by_id(mmc->chan_id);
        if(!mc)
            continue;
        auto j = cuids.index(mc->remote_uid());
        if(j == -1)
            continue;
        vc v = ret[j];
        v[M_STATUS] = "oi";
        if(mc->mms_sync_state == MMChannel::NORMAL_SEND &&
                mc->mmr_sync_state == MMChannel::NORMAL_RECV)
        {
            v[M_STATUS] = "oa";
        }
        sync_ip(mc, v);
        v[M_SENDQ_COUNT] = mc->sync_sendq.count();
    }

    ChanList cl = MMChannel::channels_by_call_type("sync");
    for(int i = 0; i < cl.num_elems(); ++i)
    {
        MMChannel *mc = cl[i];
        auto j = cuids.index(mc->remote_uid());
        if(j == -1)
            continue;
        vc v = ret[j];
        v[M_STATUS] = "ri";
        if(mc->mms_sync_state == MMChannel::NORMAL_SEND &&
                mc->mmr_sync_state == MMChannel::NORMAL_RECV)
        {
            v[M_STATUS] = "ra";
        }
        sync_ip(mc, v);
        v[M_SENDQ_COUNT] = mc->sync_sendq.count();
    }
    // the reason this looks a little goofy is because
    // i did some testing and "distinct(mid)" is a lot slower
    // for some reason than using the similar "group by mid".
    // never figured out why
    // these two are about the same performance, just one is easier to
    // understand...

    // note: for this experiment, we can't calculate this anymore, we'll need
    // different protocol for it (maybe create another small table during initial
    // sync setup or something.

#if 0
    vc res = sql_run_sql(
    "with total_mids(cnt) as (select count(*) from (select 1 from gi group by mid))"
    "select (count(*) * 100) / (select cnt from total_mids), from_client_uid from gi group by from_client_uid"
                );
    //vc res = sql_run_sql("select (count(*) * 100) / (select count(*) from (select 1 from gi group by mid)), from_client_uid from gi group by from_client_uid");
    if(!res.is_nil())
    {
    for(int i = 0; i < res.num_elems(); ++i)
    {
        vc uid = from_hex(res[i][1]);
        auto j = cuids.index(uid);
        if(j == -1)
            continue;
        vc v = ret[j];
        v[M_PERCENT_SYNCED] = res[i][0];
    }
    }
#endif

    GRTLOGVC(ret);
    return ret;
}


// this gets called when the alternate group config
// changes. we drop all the sync channels so we can
// reestablish channels in the new group.
void
drop_all_sync_calls(DH_alternate *)
{
    ChanList cl = get_all_sync_chans();
    for(int i = 0; i < cl.num_elems(); ++i)
        cl[i]->schedule_destroy(MMChannel::HARD);
}

static
void
sync_call_disposition(int call_id, int chan_id, int what, void *user_arg, const char *uid, int len_uid, const char *call_type, int len_call_type)
{

    switch(what)
    {
    case DWYCO_CALLDISP_STARTED:
        break;
    case DWYCO_CALLDISP_ESTABLISHED:
        break;
    default:
        break;

    }
}

struct local_connect_timer : public ssns::trackable
{
    local_connect_timer() : connect_timer("sync-conn-setup"){
        connect_timer.set_autoreload(1);
        connect_timer.set_interval(1000);
        connect_timer.set_oneshot(0);
        connect_timer.reset();
        connect_timer.start();
    }

    DwTimer connect_timer;

    void throttle_up()
    {
        if(connect_timer.get_interval() == 1000)
            return;
        connect_timer.stop();
        connect_timer.set_interval(1000);
        connect_timer.start();
    }


    void throttle_down()
    {
        if(connect_timer.get_interval() == 60 * 1000)
            return;
        connect_timer.stop();
        connect_timer.set_interval(60 * 1000);
        connect_timer.start();
    }

    void db_state_change(int i) {
        if(i)
        {
            throttle_up();
        }
        else
        {
            throttle_down();
        }
    }
    void local_discovery(vc uid, int added) {
        if(added)
            throttle_up();
    }
};

static local_connect_timer *lct;
static
void
throttle_up_on_setting_change(vc, vc)
{
    lct->throttle_up();
}
// this is where we look at the set of group members, and
// schedule call attempts to any that are online. throttling the
// attempt-to-connect rate is based on whether we are
// successful in at least trying connect. if we
// can't find anyone to connect to, we reduce our rate of trying
// so we can avoid using too many cpu cycles.
//
void
sync_call_setup()
{
    if(!Current_alternate)
        return;

    if(!lct)
    {
        lct = new local_connect_timer;
        Database_online.value_changed.connect_memfun(lct, &local_connect_timer::db_state_change);
        Local_uid_discovered.connect_memfun(lct, &local_connect_timer::local_discovery);
        Online_info.connect_memfun(lct, &local_connect_timer::throttle_up);
        bind_sql_section("net/", throttle_up_on_setting_change);
        bind_sql_setting("sync/eager", throttle_up_on_setting_change);
    }

    if(!lct->connect_timer.is_expired())
        return;
    lct->connect_timer.ack_expire();

    vc uids = uids_to_call();
    DwVecP<MMCall> mmcl = MMCall::calls_by_type("sync");
    for(int i = 0; i < mmcl.num_elems(); ++i)
    {
        MMCall *mmc = mmcl[i];
        uids.del(mmc->uid);
    }
    vc call_uids(VC_VECTOR);
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        if(!MMChannel::channel_by_call_type(uids[i], "sync"))
            call_uids.append(uids[i]);
    }

    // note: we only call uids that are lexicographically larger than
    // us. this is a klugey way to avoid getting two calls going between
    // two clients, one incomming and one outgoing. this isn't the best
    // necessarily because the largest guy in the list will never originate
    // a call, instead waiting for another client to start things up.
    int succ = 0;
    for(int i = 0; i < call_uids.num_elems(); ++i)
    {
        if(!Broadcast_discoveries.contains(call_uids[i]))
        {
            if(!Online.contains(call_uids[i]))
            {
                // we should throttle down the checking at this point
                // because we don't have any info that the target
                // uid is online.

                if(!Database_online)
                {
                    // there is no chance a server assisted call is
                    // going to work.
                    continue;
                }
                else
                {
                    // there is a chance they are "invisible" or just not
                    // showing up via the pal server, but for now, we just
                    // give up, since that is probably pretty rare.
                    continue;
                }
            }
            else
            {
                // we have an ip address, which might be stale, but
                // at least we know they were around recently
                if(!Database_online)
                {
                    continue;
                }
                else
                {
                    // possibly a server assisted call will work
                }
            }
        }
        else
        {
            // we have a local discovery, so keep trying it since we
            // don't need a server to set that up.
            // but don't pound it.
            if(!Database_online)
            {
                //throttle down timer
            }
            else
            {
                // possibly a server assisted call will work
            }
        }
        GRTLOG("trying sync to %s", (const char *)to_hex(call_uids[i]), 0);
        if(originate_calls(call_uids[i]))
        {
            vc pw;
            // this is ok for testing, as it will keep us from
            // screwing up and syncing with non-group members.
            // but doesn't work otherwise, since a phony could
            // connect normally and capture the hash, then present it
            // to other group members to connect. the sync protocol
            // contains a challenge to make sure both sides can
            // decrypt group messages
            if(Current_alternate)
            {
                // i'm adding the schema versions for the index
                // databases too, just to keep us from connecting
                // and being confused with message format differences
                // this should probably be fixed in some other way.
                pw = Current_alternate->hash_key_material();
                DwString p((const char *)pw, pw.len());
                p += Schema_version_hack;
                pw = vc(VC_BSTRING, p.c_str(), p.length());
            }
            else
                pw = "";
            GRTLOG("out trying sync to %s (%s)", (const char *)to_hex(call_uids[i]), (const char *)to_hex(pw));
            if(dwyco_connect_uid(call_uids[i], call_uids[i].len(), sync_call_disposition, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              (const char *)pw, pw.len(), "sync", 4, 1))
            {
                succ = 1;
            }
        }
    }
    if(!succ)
    {
        // if we didn't get at least one connection started, just throttle back
        lct->throttle_down();
    }
    //start_stalled_pulls();
}
