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
#include "snds.h"
#include "dwrtlog.h"
#include "fnmod.h"
#include "se.h"
#include "dwscoped.h"
#include "qauth.h"
#include "qmsgsql.h"
#include "xinfo.h"
#include "dwrtlog.h"

using namespace dwyco;

#ifdef LINUX
#include "miscemu.h"
#endif
#include "sepstr.h"

namespace dwyco {
int Eager_sync = 0;
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
        return 0;
    }
    close(fd);
    return 1;
}

vc
MMChannel::package_index()
{
    vc fn = sql_dump_mi();
    vc fn2 = sql_dump_mt();

    vc cmd(VC_VECTOR);
    cmd[0] = "idx";
    cmd[1] = file_to_string((const char *)fn);
    cmd[2] = file_to_string((const char *)fn2);
    remove((const char *)fn);
    remove((const char *)fn2);
    return cmd;
}

vc
MMChannel::package_next_cmd()
{
    vc next_cmd = sync_sendq.delmin();
    return next_cmd;
}

void
MMChannel::unpack_index(vc cmd)
{
    if(cmd[0] != vc("idx"))
        return;
    vc huid = to_hex(remote_uid());
    GRTLOG("unpack from %s", (const char *)huid, 0);
    DwString mifn("mi%1.sql");
    DwString favfn("fav%1.sql");

    mifn.arg((const char *)huid);
    favfn.arg((const char *)huid);
    mifn = newfn(mifn);
    favfn = newfn(favfn);
    string_to_file(cmd[1], mifn);
    string_to_file(cmd[2], favfn);
    import_remote_mi(remote_uid());
}

void
MMChannel::process_pull(vc cmd)
{
    if(cmd[0] != vc("pull"))
        return;
    vc mid = cmd[1];
    vc pri = cmd[5];
    // load the msg and attachment, and send it back as a "pull-resp"

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
    send_pull_resp(mid, from_hex(huid), body, att, pri);
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
        pull_done.emit(mid, remote_uid(), vcnil);
        return;
    }

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
            string_to_file(att, fd);
        }
        // note: this would do an se_emit to say the msg index had changed,
        // when in fact, it doesn't really change. we would not have been
        // able to do the pull if it wasn't already in some part of the index
        // we do need to make a local record of it. we can either copy it
        // from the record that induced the pull, or we can just recreate it.
        // for now, we just recreate, and do not emit the msg_update
        update_msg_idx(uid, m, 1);
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
                string_to_file(att, fd);
            }
            update_msg_idx(uid, m, 1);

    }
    sql_commit_transaction();
    se_emit_msg_pull_ok(mid, uid);
    pull_done.emit(mid, remote_uid(), vctrue);

}

void
MMChannel::process_iupdate(vc cmd)
{
    //GRTLOGVC(cmd);
    import_remote_iupdate(remote_uid(), cmd[1]);
}

void
MMChannel::process_tupdate(vc cmd)
{
    //GRTLOGVC(cmd);
    import_remote_tupdate(remote_uid(), cmd[1]);
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
    pull_target_destroyed(remote_uid());
    sql_run_sql("delete from current_clients where uid = ?1", to_hex(remote_uid()));
}

void assert_eager_pulls(MMChannel *, vc uid);

void
MMChannel::mms_sync_state_changed(enum syncstate s)
{
    vc huid = to_hex(remote_uid());

    GRTLOG("mms state to %s %d", (const char *)huid, s);
    if(s == NORMAL_SEND)
    {
        sql_run_sql("insert into current_clients values(?1)", huid);
        // note: it is better to do this once on the recv side
        // since we get the signal right after we unpack the
        // remotes latest info
//        if(Eager_sync)
//        {
//            assert_eager_pulls(this, remote_uid());
//        }
    }
    else
    {
        sql_run_sql("delete from current_clients where uid = ?1", huid);
    }
}

void
MMChannel::mmr_sync_state_changed(enum syncstate s)
{
    vc huid = to_hex(remote_uid());

    GRTLOG("mmr state to %s %d", (const char *)huid, s);
    if(s == NORMAL_RECV)
    {
        sql_run_sql("insert into current_clients values(?1)", huid);
        if(Eager_sync)
        {
            assert_eager_pulls(this, remote_uid());
        }
    }
    else
    {
        sql_run_sql("delete from current_clients where uid = ?1", huid);
    }

}

int
MMChannel::process_outgoing_sync()
{
    if(mms_sync_state == MMSS_ERR)
        return 0;
    if(!tube->can_write_data(msync_chan))
    {
        return 0;
    }

    int err;

    vc vcx;
    if(mms_sync_state == SEND_INIT)
    {
        vcx = package_index();
        destroy_signal.connect_memfun(this, &MMChannel::cleanup_pulls);
    }
    else if(mms_sync_state == MMSS_TRYAGAIN)
    {
        vcx = vcnil;
    }
    else if(mms_sync_state == NORMAL_SEND)
    {
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
    int ret;

    vc rvc;
    if((ret = tube->recv_data(rvc, msync_chan)) >= 0)
    {
        if(rvc.type() == VC_VECTOR)
        {
            // drop pings
            if(rvc[0] == vc("!"))
                return 1;
            if(mmr_sync_state == RECV_INIT)
            {
                unpack_index(rvc);
                mmr_sync_state = NORMAL_RECV;
                destroy_signal.connect_memfun(this, &MMChannel::cleanup_pulls);
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
            }
            return 1;
        }
        else
            ret = SSERR;
    }

    if(ret == SSERR)
    {
        drop_subchannel(msync_chan);
        msync_chan = -1;
        mms_sync_state = MMSS_ERR;
    }
    return 0;
}
