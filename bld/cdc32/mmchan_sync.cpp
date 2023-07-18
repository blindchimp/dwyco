#ifdef _WIN32
#ifdef _MSC_VER
#include <direct.h>
#endif
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mmchan.h"
#include "netvid.h"

#include "qmsg.h"
#include "dwrtlog.h"
#include "fnmod.h"
#include "se.h"
#include "qauth.h"
#include "qmsgsql.h"
#include "xinfo.h"
#include "dwrtlog.h"
#include "ezset.h"
#include "dhgsetup.h"
#include "pulls.h"
#include "synccalls.h"
#ifdef DWYCO_BACKGROUND_SYNC
#include <thread>
#include <future>
#include <chrono>
#endif

using namespace dwyco;
using namespace dwyco::qmsgsql;

#ifdef LINUX
#include "miscemu.h"
#endif
#include "sepstr.h"

// NOTE: when we are in eager mode, we should schedule
// a periodic "reassert" across a connection if there have been any changes
// to the global model from a connection (ie, someone says they might have
// something new.)

void
MMChannel::assert_eager_pulls()
{
    vc uid = remote_uid();
    vc huid = to_hex(uid);
    vc mids = sql_get_non_local_messages_at_uid(uid, 100);
    if(mids.is_nil())
        return;
    for(int i = 0; i < mids.num_elems(); ++i)
    {
        vc mid = mids[i];
        pulls::assert_pull(mid, uid, PULLPRI_BACKGROUND);
        if(pulls::set_pull_in_progress(mid, uid))
            send_pull(mid, PULLPRI_BACKGROUND);
    }
}

void
MMChannel::start_stalled_pulls()
{
    DwVecP<pulls> stalled_pulls = pulls::get_stalled_pulls(remote_uid());
    for(int i = 0; i < stalled_pulls.num_elems(); ++i)
    {
        stalled_pulls[i]->set_in_progress(1);
        send_pull(stalled_pulls[i]->mid, stalled_pulls[i]->pri);
    }
}

static
vc
file_to_string(DwString fn)
{
    int fd = open(fn.c_str(), O_RDONLY);
    if(fd == -1)
        return vcnil;
    DwString res;
    while(1)
    {
        char buf[32768];
        auto num = read(fd, buf, sizeof(buf));
        if(num == -1)
        {
            close(fd);
            return vcnil;
        }
        if(num == 0)
            break;
        res += DwString(buf, num);
    }
    close(fd);
    return vc(VC_BSTRING, res.c_str(), res.length());
}

static
int
string_to_file(vc str, DwString fn)
{
    int fd = open(fn.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(fd == -1)
        return 0;
    if(write(fd, (const char *)str, str.len()) != str.len())
    {
        close(fd);
        DeleteFile(fn.c_str());
        return 0;
    }
    close(fd);
    return 1;
}

vc
MMChannel::package_index()
{
    vc fn;

    try
    {
        fn = sql_dump_mi();
    }
    catch(...)
    {
        return vcnil;
    }

    vc cmd(VC_VECTOR);
    cmd[0] = "idx";
    cmd[1] = file_to_string((const char *)fn);

    vc huid = to_hex(remote_uid());
    DwString mfn = DwString("minew%1.tdb").arg((const char *)huid);
    mfn = newfn(mfn);
    if(!move_replace((const char *)fn, mfn))
        return vcnil;
    if(cmd[1].is_nil())
        return vcnil;
    create_dump_indexes(mfn);
    return cmd;
}

vc
MMChannel::package_next_cmd()
{
    vc next_cmd = sync_sendq.delmin();
    return next_cmd;
}

int
MMChannel::unpack_index(vc cmd)
{
    if(cmd[0] != vc("idx"))
        return 0;
    // deltas are coming as normal updates
    if(cmd[1].is_nil())
        return 1;

    vc huid = to_hex(remote_uid());
    GRTLOG("unpack from %s", (const char *)huid, 0);
    DwString mifn("mi%1.tdb");

    mifn.arg((const char *)huid);
    mifn = newfn(mifn);
    if(!string_to_file(cmd[1], mifn))
        return 0;
    if(!import_remote_mi(remote_uid()))
    {
        DeleteFile(mifn.c_str());
        return 0;
    }
    return 1;
}

void
MMChannel::process_pull(vc cmd)
{
    if(cmd[0] != vc("pull"))
        oopanic("pull");
    vc mid = cmd[1];
    vc pri = cmd[5];
    // load the msg and attachment, and send it back as a "pull-resp"
    // XXX note this probably needs to be done in one query, but
    // this is a weird case anywhere, where a client has sent us
    // a request thinking it is still in the global index...
    // this is kinda a race if there are tombstones propagating
    // while a bunch of pulls are getting done.
    if(!sql_is_mid_local(mid) || sql_mid_has_tombstone(mid))
    {
        send_pull_error(mid, pri);
        return;
    }
    vc huid = sql_get_uid_from_mid(mid);
    if(huid.is_nil())
    {
        send_pull_error(mid, pri);
        return;
    }

    vc body = load_body_by_id(from_hex(huid), mid);
    if(body.is_nil())
    {
        send_pull_error(mid, pri);
        return;
    }

    vc attfn = body[QM_BODY_ATTACHMENT];
    vc att;
    if(!attfn.is_nil())
    {
        DwString a("%1.usr%2%3");
        a.arg((const char *)huid, DIRSEPSTR, (const char *)attfn);
        a = newfn(a);
        att = file_to_string(a);
        if(att.is_nil())
        {
            send_pull_error(mid, pri);
            return;
        }
    }
    // XXX: this is where we would encrypt to an UNTRUSTED client
    // unless we ourselves are untrusted, then just send the ecnrypted stuff
    send_pull_resp(mid, from_hex(huid), body, att, pri);
}

void
MMChannel::pull_done(vc mid, vc remote_uid, vc success)
{
    if(success.is_nil())
    {
        pulls::pull_failed(mid, remote_uid);
        add_pull_failed(mid, remote_uid);
    }
    else
    {
        // if the pull succeeded, cancel all the
        // extant pulls in other send_qs
        pulls::deassert_pull(mid);
        ChanList cl = get_all_sync_chans();
        for(int i = 0; i < cl.num_elems(); ++i)
        {
            cl[i]->sync_sendq.del_pull(mid);
        }
        clean_pull_failed_mid(mid);
    }
}

void
MMChannel::process_pull_resp(vc cmd)
{
    // here is where we insert the fetched message into our
    // local model (which automatically gets put into the global
    // model held here.)
    // WARNING: the uid here is the uid associated with the message
    // we fetched, NOT the uid of the client that sent us the response.

    vc mid = cmd[1];
    vc uid = cmd[2];
    vc body = cmd[3];
    vc att = cmd[4];

    if(uid.is_nil())
    {
        pull_done(mid, remote_uid(), vcnil);
        return;
    }

    // XXX: HERE IS WHERE WE DECRYPT FROM AN UNTRUSTED PULL
    // unless we ourselves are untrusted, in which case we just
    // save it as is. note problem below...
    vc m = body;
    DwString udir;
    init_msg_folder(uid, &udir);
    sql_start_transaction();
    if(body[QM_BODY_SENT].is_nil())
    {
        DwString tn = udir;
        tn += DIRSEPSTR;
        tn += (const char *)mid;
        tn += ".bod";
        if(!save_info(m, tn.c_str(), 1))
        {
            oopanic("cant save");
        }
        vc aname = m[QM_BODY_ATTACHMENT];
        if(!aname.is_nil())
        {
            DwString fd = udir;
            fd += DIRSEPSTR;
            fd += (const char *)aname;
            if(!string_to_file(att, fd))
                oopanic("cant save att");
        }
        // if the msg is already in the list of messages from the
        // server, nuke it here so it won't show up in the
        // message models as if it needs fetching.
        delete_msg2(mid);

        // note: this would do an se_emit to say the msg index had changed,
        // when in fact, it doesn't really change. we would not have been
        // able to do the pull if it wasn't already in some part of the index
        // we do need to make a local record of it. we can either copy it
        // from the record that induced the pull, or we can just recreate it.
        // for now, we just recreate, and do not emit the msg_update
        //
        // note: if we are untrusted ourselves, the index has to be
        // derived differently. since we don't really need most of the info
        // to sync, maybe some abbreviated info can be sent from the
        // other client.
        if(!update_msg_idx(uid, m, 1))
        {
            // managed to save message, but indexing failed,
            // schedule a reindex
        }
    }
    else
    {
            DwString tn = udir;
            tn += DIRSEPSTR;
            tn += (const char *)mid;
            tn += ".snt";
            if(!save_info(m, tn.c_str(), 1))
            {
                oopanic("cant save");
            }
            vc aname = m[QM_BODY_ATTACHMENT];
            if(!aname.is_nil())
            {
                DwString fd = udir;
                fd += DIRSEPSTR;
                fd += (const char *)aname;
                if(!string_to_file(att, fd))
                    oopanic("cant save att");
            }
            if(!update_msg_idx(uid, m, 1))
            {
                // managed to save message, but indexing failed,
                // schedule a reindex
            }

    }
    sql_commit_transaction();
    se_emit_msg_pull_ok(mid, uid);
    pull_done(mid, remote_uid(), vctrue);

}

void
MMChannel::process_iupdate(vc cmd)
{
    //GRTLOGVC(cmd);
    vc mid = import_remote_iupdate(remote_uid(), cmd[1]);
    // this is the piecemeal restart of an assert
    if(!mid.is_nil() && pulls::set_pull_in_progress(mid, remote_uid()))
    {
        send_pull(mid, PULLPRI_NORMAL);
    }
}

void
MMChannel::process_tupdate(vc cmd)
{
    //GRTLOGVC(cmd);
    import_remote_tupdate(remote_uid(), cmd[1]);
}

void
MMChannel::process_syncpoint(vc cmd)
{
    import_new_syncpoint(remote_uid(), cmd[1]);
}

void
MMChannel::send_pull(vc mid, int pri)
{
    vc cmd(VC_VECTOR);
    cmd[0] = "pull";
    cmd[1] = mid;
    cmd[2] = pri;
    sync_sendq.append(cmd, pri);
}

void
MMChannel::send_pull_resp(vc mid, vc uid, vc msg, vc att, vc pri)
{
    vc cmd(VC_VECTOR);
    cmd[0] = "pull-resp";
    cmd[1] = mid;
    cmd[2] = uid;
    cmd[3] = msg;
    cmd[4] = att;
    sync_sendq.append(cmd, pri);
}

void
MMChannel::send_pull_error(vc mid, vc pri)
{
    send_pull_resp(mid, vcnil, vcnil, vcnil, pri);
}

void pull_target_destroyed(vc uid);
void
MMChannel::cleanup_pulls(int myid)
{
    vc uid = remote_uid();
    vc huid = to_hex(uid);
    pull_target_destroyed(uid);
    sql_run_sql("delete from current_clients where uid = ?1", huid);
    sql_run_sql("delete from midlog where to_uid = ?1", huid);
    sql_run_sql("delete from taglog where to_uid = ?1", huid);
}

void
MMChannel::mms_sync_state_changed(enum syncstate s)
{
    vc huid = to_hex(remote_uid());

    GRTLOG("mms state to %s %d", (const char *)huid, s);
    if(s == NORMAL_SEND)
    {
        sql_run_sql("insert into current_clients values(?1)", huid);
        // note: it is better to do assert eager pulls once on the recv side
        // since we get the signal right after we unpack the
        // remote client's latest info
        Group_uids.add(remote_uid());
    }
    else
    {
        sql_run_sql("delete from current_clients where uid = ?1", huid);
    }
}

// note: when the mmr_sync_state changes to "NORMAL_RECV" we have
// processed the remote client's index and merged it with our index,
// so anything that depends on that is good to do here.
void
MMChannel::mmr_sync_state_changed(enum syncstate s)
{
    vc huid = to_hex(remote_uid());

    GRTLOG("mmr state to %s %d", (const char *)huid, s);
    if(s == NORMAL_RECV)
    {
        sql_run_sql("insert into current_clients values(?1)", huid);
        clean_pull_failed_uid(remote_uid());
        if((int)get_settings_value("sync/eager") == 1)
        {
            assert_eager_pulls();
        }
        else
        {
            start_stalled_pulls();
        }
    }
    else
    {
        sql_run_sql("delete from current_clients where uid = ?1", huid);
    }

}

void
MMChannel::eager_pull_processing()
{
    if((int)get_settings_value("sync/eager") == 0)
        return;

    if(!eager_pull_timer.is_running())
    {
        eager_pull_timer.set_interval(10000);
        eager_pull_timer.set_autoreload(1);
        eager_pull_timer.start();
    }
    if(eager_pull_timer.is_expired())
    {
        eager_pull_timer.ack_expire();
        if((sync_sendq.count() == 0) &&
                tube && msync_state == MEDIA_SESSION_UP && !tube->has_data(msync_chan))
        {
            // only assert if the incoming channel appears to be empty
            // as well. this gives us a chance to process the results of
            // all the previous requests, so we can make progress as the
            // msgs are transferred and saved here (the index will be updated
            // and we will avoid a lot of re-asserts of things we have already
            // seen.)
            assert_eager_pulls();
        }
    }
}

void
MMChannel::throttle_downstream_timer(vc)
{
    if(downstream_timer.get_interval() != 5000)
    {
        downstream_timer.stop();
        downstream_timer.set_interval(5000);
    }
    downstream_timer.start();

}

int
MMChannel::process_outgoing_sync()
{
    if(mms_sync_state == MMSS_ERR)
        return 0;
    if(mms_sync_state == SEND_DELTA_OK)
    {
        // generate_delta created entries on the midlog that can get sent
        // normally without requiring a re-init on the receiver. so we
        // transition straight into NORMAL state.
        destroy_signal.connect_memfun(this, &MMChannel::cleanup_pulls, 1);
        Qmsg_update.connect_memfun(this, &MMChannel::throttle_downstream_timer, 1);
        throttle_downstream_timer(vcnil);
        // tell the other side there is no index to unpack
        vc vcx(VC_VECTOR);
        vcx[0] = "idx";
        vcx[1] = vcnil;
        sync_sendq.append(vcx, PULLPRI_INIT);
        mms_sync_state = NORMAL_SEND;
        return 1;
    }

    if(!tube->can_write_data(msync_chan))
    {
        return 0;
    }

    int err;

    vc vcx;
    if(mms_sync_state == SEND_INIT)
    {
#ifdef DWYCO_BACKGROUND_SYNC
        if(package_index_future == nullptr)
        {
            auto f = new std::future<vc>;
            package_index_future = f;
            *f = std::async(std::launch::async, &MMChannel::package_index, this);
        }
        if(package_index_future->wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
        {
            return 0;
        }
        vcx = package_index_future->get();
        delete package_index_future;
        package_index_future = nullptr;
#else
        vcx = package_index();
#endif
        // note: this probably needs to be a unique signal, because the
        // the receive part sets this up too
        if(vcx.is_nil())
            return 0;
        destroy_signal.connect_memfun(this, &MMChannel::cleanup_pulls, 1);
        Qmsg_update.connect_memfun(this, &MMChannel::throttle_downstream_timer, 1);
    }
    else if(mms_sync_state == MMSS_TRYAGAIN)
    {
        vcx = vcnil;
    }
    else if(mms_sync_state == NORMAL_SEND)
    {
//        if(!downstream_timer.is_running())
//        {
//            downstream_timer.set_interval(5000);
//            downstream_timer.set_autoreload(0);
//            downstream_timer.start();
//        }
        if(downstream_timer.is_expired())
        {
            downstream_timer.ack_expire();
            vc ds = package_downstream_sends(remote_uid());
            if(!ds.is_nil())
            {
                // NOTE: *may* want to consider sending tag updates
                // before other updates (like pulls) since they are
                // likely to be small and putting them behind other updates
                // may cause them to never be updated in a timely manner
                // likewise with pulls that are initiated from the
                // model lookup, if we are doing eager updating.
                for(int i = 0; i < ds.num_elems(); ++i)
                    sync_sendq.append(ds[i], PULLPRI_NORMAL);
            }
            else
            {
                downstream_timer.stop();
            }
        }
        eager_pull_processing();

        vcx = package_next_cmd();
        if(vcx.is_nil())
            return 0;
    }
    else
    {
        oopanic("bad mm state");
    }

    if((err = tube->send_data(vcx, msync_chan, VIDEO_CHANNEL)) == SSERR)
    {
        drop_subchannel(msync_chan);
        msync_chan = -1;
        mms_sync_state = MMSS_ERR;
        return 0;
    }
    else if(err == SSTRYAGAIN)
    {
        mms_sync_state = MMSS_TRYAGAIN;
        return 0;
    }

    GRTLOG("msync output ok %d %s", myid, (const char *)to_hex(remote_uid()));
    if(mms_sync_state == NORMAL_SEND)
    {
        GRTLOGVC(vcx);
    }

    mms_sync_state = NORMAL_SEND;

    sproto *s = simple_protos[msync_chan];
    s->timeout.load(VIDEO_IDLE_TIMEOUT);
    s->timeout.start();

    return 1;
}

int
MMChannel::process_incoming_sync()
{
    try
    {
        int ret;
        vc rvc;
        ret = tube->recv_data(rvc, msync_chan);
        if(ret == SSERR)
            throw -1;
        if(ret == SSTRYAGAIN)
            return 0;
        if(rvc.type() != VC_VECTOR)
            throw -1;
        sync_pinger.load(2 * VIDEO_IDLE_TIMEOUT);
        sync_pinger.start();

        // drop pings
        if(rvc[0] == vc("!"))
            return 1;
        if(mmr_sync_state == RECV_INIT)
        {
            destroy_signal.connect_memfun(this, &MMChannel::cleanup_pulls, 1);
#ifdef DWYCO_BACKGROUND_SYNC
            if(unpack_index_future == nullptr)
            {
                unpack_index_future = new std::future<int>();
                *unpack_index_future = std::async(std::launch::async, &MMChannel::unpack_index, this, rvc);
            }
            if(unpack_index_future->wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
            {
                return 0;
            }
            int ret = unpack_index_future->get();
            delete unpack_index_future;
            unpack_index_future = nullptr;
            if(!ret)
                throw -1;
#else
            if(!unpack_index(rvc))
                throw -1;
#endif
            mmr_sync_state = NORMAL_RECV;
        }
        else if(mmr_sync_state == NORMAL_RECV)
        {
            GRTLOG("normal sync %d %s", myid, (const char *)to_hex(remote_uid()));
            GRTLOGVC(rvc);
            vc cmd = rvc[0];
            if(cmd == vc("pull"))
                process_pull(rvc);
            else if(cmd == vc("pull-resp"))
                process_pull_resp(rvc);
            else if(cmd == vc("iupdate"))
            {
                process_iupdate(rvc);
            }
            else if(cmd == vc("tupdate"))
            {
                process_tupdate(rvc);
            }
            else if(cmd == vc("sync"))
            {
                process_syncpoint(rvc);
            }
        }
        return 1;
    }
    catch(...)
    {
        drop_subchannel(msync_chan);
        msync_chan = -1;
        mms_sync_state = MMSS_ERR;
        schedule_destroy(HARD);
    }
    return 0;
}
