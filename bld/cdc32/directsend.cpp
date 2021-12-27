
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "directsend.h"
#include "mmcall.h"
#include "ta.h"
#include "qdirth.h"
#include "qauth.h"
#include "calllive.h"
#include "dwrtlog.h"
#include "se.h"
#include "fnmod.h"
#include "qmsg.h"
#include "sepstr.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef LINUX
#include <unistd.h>
#endif

#define CHANNEL_SETUP_TIMEOUT (1000 * 4)
#define XFER_WATCHDOG_TIMEOUT (1000 * 20)

using namespace dwyco;
DwQueryByMember<DirectSend> DirectSend::Qbm;

DirectSend::DirectSend(const DwString& fn) :
    vp(this)
{
    qfn = fn;
    inprogress = 0;
    cancel_op = 0;
    xfer_chan_id = -1;
    send_chan_id = -1;
    xfer_channel = 0;
    dont_save_sent = 0;
    Qbm.add(this);
}

DirectSend::~DirectSend()
{
    vp.invalidate();
    Qbm.del(this);
}

static
void
async_delete(vc, void *, vc, ValidPtr vp)
{
    if(!vp.is_valid())
        return;
    delete (DirectSend *)(void *)vp;
}

static
void
delete_later(DirectSend *d)
{
    d->disconnect_all();
    dirth_q_local_action(vc(VC_VECTOR), QckDone(async_delete, 0, vcnil, d->vp));
}

static
void
send_done(vc m, void *, vc, ValidPtr vp)
{
    if(!vp.is_valid())
        return;
    DirectSend *q = (DirectSend *)(void *)vp;
    if(!q)
        return;
    if(q->cancel_op)
    {
        delete_later(q);
        return;
    }
    if(m[1].is_nil())
    {
        move_back_to_outbox(q->qfn);
        q->fail();
    }
    else
    {
        q->cleanup_after_send();
        q->succeed();
    }
    delete_later(q);
}

static
MMChannel *
get_send_channel(vc uid)
{
    MMChannel *mp;

    DwVecP<MMCall> ret = MMCall::MMCalls_qbm.query_by_member(uid, &MMCall::uid);
    for(int i = 0; i < ret.num_elems(); ++i)
    {
        if(!ret[i]->established)
        {
            continue;
        }
        if(!ret[i]->msg_chan)
            continue;
        mp = MMChannel::channel_by_id(ret[i]->chan_id);
        if(!mp)
        {

            return 0;
        }
        // mp is parent channel for send
        if(!mp->tube)
        {

            return 0;
        }
        if(mp->remote_cfg.is_nil())
        {

            return 0;
        }
        return mp;
    }
    //if(ret.num_elems() == 0)
    //{
    // no call object, maybe we are on the receiving end, where no
    // call object exists, just see if there is a message channel that exists
    // that we can use
    mp = MMChannel::already_connected(uid, 1);
    if(mp && mp->tube && !mp->remote_cfg.is_nil())
        return mp;
    //}
    return 0;
}

void
DirectSend::send_direct()
{
    vc v(VC_VECTOR);

    MMChannel *send_channel = get_send_channel(uid);
    if(!send_channel)
    {
        TRACK_ADD(DS_null_send_chan, 1);
        fail();
        return;
    }
    QckDone d(send_done, 0, vcnil, vp);
    d.set_timeout(10);
    d.type = ReqType("msg", ++Serial);
    v[0] = d.type.response_type();
    v[1] = msg_to_send;
    v[2] = small_attachment;
    Waitq.append(d);
    send_channel->send_ctrl(v);
    send_chan_id = send_channel->myid;

    TRACK_ADD(DS_send_setups, 1);

}

static void
eo_direct_xfer(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->timer1.stop();
    if(!vp.is_valid())
        return;
    DirectSend *q = (DirectSend *)(void *)vp;
    // if cancel, avoid signals and whatnot, since we are
    // replacing that stuff with cancel signals and ops
    if(q->cancel_op)
        return;
    if(mc->xfer_failed || mc->connect_failed)
    {
        // if it was declined, don't give this option,
        // just drop it.
        if(mc->direct_zap_attachment_declined)
        {
            TRACK_ADD(DS_declined, 1);
            DeleteFile(q->qfn.c_str());
            if(q->actual_filename.length() > 0)
                DeleteFile(q->actual_filename.c_str());
            q->fail();
            return;
        }

        // switch to try and send via server, note problem so
        // we don't keep trying
        No_direct_att.add(q->msg_to_send[0][0]);
        if(mc->xfer_failed)
        {
            TRACK_ADD(DS_direct_xfer_failed, 1);
        }
        else if(mc->connect_failed)
        {
            TRACK_ADD(DS_direct_connect_failed, 1);
        }
        q->fail();
        return;
    }
    TRACK_ADD(DS_direct_xfer_ok, 1);
    q->xfer_channel = 0;
    q->send_direct();
}

static void
set_status(MMChannel *mc, vc msg, void *, ValidPtr vp)
{
    mc->timer1.stop();
    mc->timer1.load(XFER_WATCHDOG_TIMEOUT);
    mc->timer1.start();
    if(!vp.is_valid())
        return;
    DirectSend *q = (DirectSend *)(void *)vp;
    if(q)
    {
        int e = mc->expected_size;
        if(e == 0)
            e = 1;
        int p = (int)(((double)mc->total_got * 100) / e);
        q->status_sig.emit(q->qfn, q->uid, (const char *)msg, p);
    }
}

static void
xfer_chan_call_succeeded(MMChannel *mc, int chan, vc, void *, ValidPtr)
{
    // this is a little dicey, since we don't know how long the transfer is
    // going to take. ideally we would like to set a timeout so that if the
    // protocol didn't progress at all, we could abort and try the next step.
    // but i'll have to look into that more.
    mc->timer1.stop();
    // this is a bit of a kluge... we know we have status reporting
    // set up, and use that as a watchdog on the transfer
    mc->timer1.reset();
    mc->timer1.set_oneshot(1);
    mc->timer1.load(XFER_WATCHDOG_TIMEOUT);

    mc->timer1.start();
    // timer1 callback still set to xfer_chan_setup_timeout
}

static void
xfer_chan_setup_timeout(MMChannel *mc, vc arg1, void *arg2, ValidPtr vp)
{
    eo_direct_xfer(mc, arg1, arg2, vp);
}

int
DirectSend::load_small_attachment()
{
    if(actual_filename.length() == 0)
        return 0;
    struct stat sb;
    if(stat(actual_filename.c_str(), &sb) == -1)
        return 0;
    if(sb.st_size > 20 * 1024)
        return 0;

    FILE *f = fopen(actual_filename.c_str(), "rb");
    if(!f)
        return 0;
    char *buf = new char[sb.st_size];
    if(fread(buf, sb.st_size, 1, f) != 1)
    {
        delete [] buf;
        fclose(f);
        return 0;
    }

    small_attachment = vc(VC_BSTRING, buf, sb.st_size);
    delete [] buf;
    fclose(f);
    TRACK_ADD(DS_small_att, 1);
    return 1;

}

void
DirectSend::send_with_attachment()
{
    MMChannel *mp;
    mp = get_send_channel(uid);
    if(!mp)
    {
        fail();
        return;
    }

    if(load_small_attachment())
    {
        send_direct();
        return;
    }
    // for now, if there is an attachment to send, but it is a
    // proxy connection, just fail so the msg can go via regular
    // server channel. this should be fixed eventually.
    if(!mp->proxy_info.is_nil())
    {
        vc r = msg_to_send[QQM_RECIP_VEC][0];

        se_sig.emit(SE_MSG_SEND_FAIL, qfn, r);
        // hack, undo the direct send block we just put on
        No_direct_msgs.del(uid);

        delete_later(this);
        return;
    }

    MMChannel *m = new MMChannel;
    m->tube = new DummyTube;
    DwString remote((const char *)mp->tube->remote_addr_ctrl());
    int c = remote.find(":");
    if(c != DwString::npos)
        remote.remove(c);
    m->tube->connect(remote.c_str(), 0, 0);
    m->start_service();

    int chan = -1;
    int a;
    if((a = m->tube->gen_channel(mp->remote_listening_port(), chan)) == SSERR)
    {
        TRACK_ADD(DS_direct_xfer_gen_chan_fail, 1);
        m->schedule_destroy(MMChannel::HARD);
        fail();
    }
    else
    {
        TRACK_ADD(DS_direct_xfer_setups, 1);
        m->remote_filename = file_basename;
        m->local_filename = file_basename;
        m->zap_send = 1;
        m->destroy_callback = eo_direct_xfer;
        m->dcb_arg2 = 0;
        m->dcb_arg3 = vp;
        m->status_callback = set_status;
        m->scb_arg1 = 0;
        m->scb_arg2 = vp;
        m->set_progress_status("Sending audio/video...");
        m->init_config(1);
        m->recv_matches(mp->remote_cfg);
        m->session_id = mp->session_id;

        m->connect_failed = 1;

        // start a short timer, since the builtin
        // connection timer in the tcp stack is
        // really too long
        //m->timer1.set_autoreload(0);
        m->timer1.set_oneshot(1);
        m->timer1.load(CHANNEL_SETUP_TIMEOUT);
        m->timer1.reset();
        m->timer1.start();
        m->timer1_callback = xfer_chan_setup_timeout;
        m->t1cb_arg3 = vp;
        m->chan_established_callback = xfer_chan_call_succeeded;

        m->set_string_id("<<Zap Msg Send>>");
        m->start_service();
        xfer_channel = m;
        xfer_chan_id = m->myid;

        m->agreed_key = mp->agreed_key;
        sproto *s = new sproto(chan, file_send, m->vp);

        // note: we don't need to incur extra encryption
        // cost here because the channel is already encrypted.

        m->simple_protos[chan] = s;
        s->start();

    }


}

void
DirectSend::cleanup_after_send()
{
    DwString tmpfn = qfn;
    tmpfn += ".tmp";
    move_in_progress(qfn, tmpfn);
    if(!dont_save_sent)
    {
        local_mid = gen_random_filename();
        do_local_store(tmpfn.c_str(), local_mid.c_str());
    }
    DeleteFile(newfn(tmpfn).c_str());
    if(actual_filename.length() > 0)
        DeleteFile(actual_filename.c_str());
}

static
void
direct_send_callback(MMCall *, int status, void *user_arg, ValidPtr)
{
    // notify any existing objects that are waiting to try to send
    // via this new channel of the outcome of the attempt


}

void
DirectSend::call_disposition(MMCall *, int what, void *, ValidPtr vp)
{
    switch(what)
    {
    case MMCALL_STARTED:
        break;
    case MMCALL_ESTABLISHED:
        start_send();
        break;
    default:
        // something didn't happen to get us connected, so we are
        // left without a way to send. just signal end, and the
        // whoever is listening will have to arrange for another
        // delivery method
        fail();
        break;

    }
}

// returns -1 if the message cannot be sent because it can't be
// found or it is corrupt or something. the message is deleted
// in this case.
// returns 0 if the direct-send state machine is already running
// or the the machine cannot be started for some transient reason.
// returns 1 if the message is q'd to the inprogress box, and the
// direct send state machine is started.
int
DirectSend::send_message()
{
    if(inprogress)
        return 0;
    DwVecP<DirectSend> dsv = DirectSend::Qbm.query_by_member(qfn, &DirectSend::qfn);
    if(dsv.num_elems() > 1)
        return 0;
    if(!load_info(msg_to_send, qfn.c_str()))
    {
        if(actual_filename.length() > 0)
            DeleteFile(actual_filename.c_str());
        TRACK_ADD(DS_bad_load, 1);
        return -1;
    }
    if(!msg_to_send[QQM_MSG_VEC][QQM_BODY_ATTACHMENT].is_nil())
    {
        file_basename = msg_to_send[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
        actual_filename = newfn(file_basename);
    }

    if(!valid_qd_message(msg_to_send))
    {
        DeleteFile(qfn.c_str());
        if(actual_filename.length() > 0)
            DeleteFile(actual_filename.c_str());
        TRACK_ADD(DS_invalid, 1);

        return -1;
    }
    inprogress = 1;

    DwString ipn("inprogress" DIRSEPSTR "");
    ipn += qfn;
    //DeleteFile(newfn(ipn).c_str());
    if(!move_replace(newfn(qfn), newfn(ipn)))
        return 0;

    // message is now squirreled away in the inprogress box.
    // if this function returns 0 from this point on, it is
    // up to the caller to decide what to do with the message.
    // usually it would move it from inprogress straight into the
    // outbox for a retry via the server.
    // if the program crashes, the default is for all inprogress
    // msgs to be put into the outbox, which will be retried
    // via the server automatically.

    uid = msg_to_send[QQM_RECIP_VEC][0];
    dont_save_sent = msg_to_send[QQM_SAVE_SENT].is_nil();

    if(uid == My_UID && actual_filename.length() != 0)
    {
        // special case this for now... in the past, this
        // all worked just fine going thru the normal
        // mechanisms, but with direct attachments, the
        // file handling needs to be changed to allow
        // everything to come out right, which is too
        // hard at the moment.
        init_msg_folder(My_UID);
        cleanup_after_send();
        // let the caller know as if the message was delivered
        // the normal way
        start();
        succeed();
        TRACK_ADD(DS_self_send, 1);
#if 0
        msgbox("Message sent locally. Find it by: checking \"show old\", then "
               "check \"show messages you sent\", then select your name.", "Note", MB_OK);

#endif
        return 1;
    }



    // just return ok if we are in the process of setting up a direct
    // channel, or the channel is already set up
    DwVecP<MMCall> ret = MMCall::MMCalls_qbm.query_by_member(uid, &MMCall::uid);
    for(int i = 0; i < ret.num_elems(); ++i)
    {
        if(ret[i]->msg_chan)
        {
            //note: probably should set up signals for this object
            // since it will eventually try to send on the channel
            if(ret[i]->established)
            {
                TRACK_ADD(DS_dchan_reuse, 1);
                start_send();
                return 1;
            }
            TRACK_ADD(DS_dchan_not_established, 1);
            ret[i]->call_sig.connect_memfun(this, &DirectSend::call_disposition, 1);
            return 1;
        }
    }
    // make sure to check just for msg channels
    MMChannel *send_channel = MMChannel::already_connected(uid, 1);
    if(send_channel == 0)
    {
        int prim, sec, pal;
        unsigned long ip = 0;
        int can_do_direct = 0;


        // note: by not checking "online" state from pal server, we may
        // end up trying to contact someone that was in the chat server
        // and visible (since we captured their ip.) this is a bit of
        // an info leak, but one that isn't too bad.
        if((ip = uid_to_ip(uid, can_do_direct, prim, sec, pal)) == 0 ||
                !can_do_direct)
        {
            GRTLOG("ip lookup (cando %d) ip %08x", can_do_direct, ip);
            GRTLOG("uid online %d", uid_online(uid), 0);
            GRTLOG("uid %s", (const char *)to_hex(uid), 0);
            if(Track_stats)
            {
                if(!uid_online(uid))
                {
                    TRACK_ADD(DS_not_online, 1);
                }
                else if(ip == 0)
                {
                    TRACK_ADD(DS_no_ip, 1);
                }
                else if(!can_do_direct)
                {
                    TRACK_ADD(DS_no_direct, 1);
                }
            }
            // note: without at least an ip address, we don't try
            // to send it direct. we could send via server anyway
            // but that would be an info leak if the person wanted to
            // stay invisible. have to think about it.
            return 0;
        }

        TRACK_ADD(DS_direct_setup, 1);
        vc host;

        struct in_addr in;
        in.s_addr = ip;
        host = inet_ntoa(in);

        MMCall *mmc = new MMCall(uid, host, prim, vcnil, 1, 0, direct_send_callback, 0, vp);
        mmc->call_sig.connect_memfun(this, &DirectSend::call_disposition);
        if(!mmc->start_call(MEDIA_TCP_VIA_PROXY))
        {
            delete mmc;
            return 0;
        }
        return 1;
    }
    else
    {
        // we might have received a call from them and have a call channel
        // set up.
        TRACK_ADD(DS_use_slave, 1);
        start_send();
        return 1;

    }
    return 1;
}

void
DirectSend::start_send()
{
    start();
    if(actual_filename.length() > 0)
        send_with_attachment();
    else
        send_direct();

}

// this cancel obliterates the message so it is never sent.
// not sure if this is right, but maybe need to provide a "stop sending"
// version too which doesn't delete the message.
void
DirectSend::cancel()
{
    cancel_op = 1;
    if(xfer_channel)
    {
        // inhibit destroy callback so it doesn't emit signals
        xfer_channel->destroy_callback = 0;
        MMChannel::synchronous_destroy(xfer_chan_id);
        xfer_channel = 0;
    }
    dirth_cancel_callbacks(send_done, vp, ReqType());
    dont_save_sent = 1;
    cleanup_after_send();
    canceled();
    delete_later(this);
}

void
DirectSend::fail()
{
    vc r = msg_to_send[QQM_RECIP_VEC][0];

    se_sig.emit(SE_MSG_SEND_FAIL, qfn, r);
    // any failure, we close it down
    MMChannel *m = MMChannel::channel_by_id(send_chan_id);
    if(m)
        m->schedule_destroy(MMChannel::HARD);
    delete_later(this);
    TRACK_ADD(DS_fail, 1);

}

void
DirectSend::succeed()
{
    vc r = msg_to_send[QQM_RECIP_VEC][0];

    se_sig.emit(SE_MSG_SEND_SUCCESS, qfn, r);
    // note: for direct sends, when we get a response, we know the recipient got
    // the message. note also, that doesn't mean the message will ever be
    // seen by the recipient... it might be ignored or whatever.
    se_sig.emit(SE_MSG_SEND_DELIVERY_SUCCESS, local_mid, r);
    delete_later(this);
    TRACK_ADD(DS_succeed, 1);

}

void
DirectSend::start()
{
    vc r = msg_to_send[QQM_RECIP_VEC][0];
    se_sig.emit(SE_MSG_SEND_START, qfn, r);

}

void
DirectSend::canceled()
{
    vc r = msg_to_send[QQM_RECIP_VEC][0];

    se_sig.emit(SE_MSG_SEND_CANCELED, qfn, r);
    delete_later(this);

}
