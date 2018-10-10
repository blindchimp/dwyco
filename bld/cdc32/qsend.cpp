
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "qsend.h"
#include "mmchan.h"
#include "qmsg.h"
#include "qdirth.h"
#include "se.h"
#include "dwlog.h"
#include "fnmod.h"
#include "dirth.h"
#include "filetube.h"
#include "xinfo.h"
#include "doinit.h"
#include "pkcache.h"
#include "vccrypt2.h"
#include "vcudh.h"
#include "sepstr.h"
#include "ta.h"
#include "dhgsetup.h"
#ifdef WIN32
#include <io.h>
#endif
extern vc My_UID;

// these are a bit longer because we are at the end of
// the road... either we send or the message gets stuck
// which can happen on really slow mobile connections
#define CHANNEL_SETUP_TIMEOUT (1000 * 15)
#define XFER_WATCHDOG_TIMEOUT (1000 * 60)

namespace dwyco {

// note: if you set both Force and Avoid, nothing will get sent
// to the server, messages will still be q'd up, just never sent.
int Avoid_pk;
int Force_pk;
dwyco::DwQueryByMember<DwQSend> DwQSend::Qbm;

// send a q'd message

DwQSend::DwQSend(const DwString &qfn) :
    vp(this)
{
    inprogress = 0;
    cancel_op = 0;
    dont_save_sent = 0;
    this->qfn = qfn;
    xfer_chan_id = -1;
    att_size = 0;
    has_att = 0;
    force_encryption = 0;
    Qbm.add(this);
}

DwQSend::~DwQSend()
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
    delete (DwQSend *)(void *)vp;
}

static
void
delete_later(DwQSend *d)
{
    dirth_q_local_action(vc(VC_VECTOR), QckDone(async_delete, 0, vcnil, d->vp));
}

void
DwQSend::fail()
{
    vc r = msg[QQM_RECIP_VEC][0];
    sig.emit(0, DWYCO_MSG_SEND_FAILED, (const char *)r, r.len(), 0, mscb_arg1);
    se_sig.emit(SE_MSG_SEND_FAIL, qfn, r);
    TRACK_ADD(QS_fail, 1);

}

void
DwQSend::succeed()
{
    vc r = msg[QQM_RECIP_VEC][0];
    sig.emit(0, DWYCO_MSG_SEND_OK, (const char *)r, r.len(), 0, mscb_arg1);
    se_sig.emit(SE_MSG_SEND_SUCCESS, qfn, r);
    TRACK_ADD(QS_succeed, 1);

}

void
DwQSend::start()
{
    vc r = msg[QQM_RECIP_VEC][0];
    se_sig.emit(SE_MSG_SEND_START, qfn, r);
    TRACK_ADD(QS_start, 1);

}

void
DwQSend::canceled()
{
    vc r = msg[QQM_RECIP_VEC][0];
    se_sig.emit(SE_MSG_SEND_CANCELED, qfn, r);
    TRACK_ADD(QS_cancel, 1);

}

void
DwQSend::save_aux()
{
    vc v(VC_VECTOR);
    v[0] = start_time;
    v[1] = try_count;
    v[2] = prev_ip;
    v[3] = prev_port;
    DwString fn = fn_base_wo_extension(qfn);
    fn += ".aux";
    save_info(v, fn.c_str());
}

int
DwQSend::load_aux()
{
    vc v;

    DwString fn = fn_base_wo_extension(qfn);
    fn += ".aux";
    if(load_info(v, fn.c_str()))
    {
        start_time = v[0];
        try_count = v[1];
        prev_ip = v[2];
        prev_port = v[3];
        return 1;
    }
    return 0;
}

void
DwQSend::remove_aux()
{
    DwString fn = fn_base_wo_extension(qfn);
    fn += ".aux";
    DeleteFile(newfn(fn).c_str());
}



void
DwQSend::qd_send_done(vc m, void *, vc, ValidPtr vp)
{

    if(!vp.is_valid())
        return;
    DwQSend *qs = reinterpret_cast<DwQSend *>((void *)vp);
    qs->inprogress = 0;
    if(qs->cancel_op)
    {
        delete_later(qs);
        return;
    }
    //vc afn = qs->att_basename;
    if(m[1].is_nil())
    {
        // move message back to outbox
        move_back_to_outbox(qs->qfn);
        qs->fail();
    }
    else
    {
        // newer servers return the server assigned mid so we can
        // associate a delivery response

        if(m[3].type() == VC_VECTOR && !m[3][0].is_nil())
            qs->delivered_mid = m[3][0];
        Log->make_entry("Sent q'd message.");
        DwString tmpfn = qs->qfn;
        tmpfn += ".tmp";
        move_in_progress(qs->qfn, tmpfn);
        if(!qs->dont_save_sent)
        {
            do_local_store(tmpfn.c_str(), qs->delivered_mid);
        }

        DeleteFile(newfn(tmpfn).c_str());

        // note: need to clean up both enc and non-enc attachments here
        if(qs->att_actual_fn.length() > 0)
            DeleteFile(qs->att_actual_fn.c_str());
        if(qs->alternate_att_actual_fn.length() > 0)
            DeleteFile(qs->alternate_att_actual_fn.c_str());

        DwString efn = qs->qfn;
        efn += ".enc";
        DeleteFile(newfn(efn).c_str());
        qs->remove_aux();
        qs->succeed();
    }
    delete_later(qs);
}

void
DwQSend::do_store()
{
    if(!att_basename.is_nil())
    {
        vc loc(VC_VECTOR);
        loc[0] = ip;
        loc[1] = port + vc(1);
        msg[QQM_MSG_VEC][QQM_BODY_ATTACHMENT_LOCATION] = loc;

        if(!emsg.is_nil())
            emsg[QQM_MSG_VEC][QQM_BODY_ATTACHMENT_LOCATION] = loc;
    }
    send_done_future = QckDone(qd_send_done, 0, vcnil, vp);
    vc to_send = emsg.is_nil() ? msg[QQM_MSG_VEC] : emsg[QQM_MSG_VEC];
    to_send[QQM_BODY_ESTIMATED_SIZE] = att_size;
    dirth_send_store(My_UID, msg[0], to_send, send_done_future);
}


void
DwQSend::eo_qd_xfer(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->timer1.stop();
    if(!vp.is_valid())
        return;
    DwQSend *qs = reinterpret_cast<DwQSend *>((void *)vp);
    if(!qs)
        return;
    if(qs->cancel_op)
        return;
    qs->xfer_chan_id = -1;
    if(mc->xfer_failed || mc->connect_failed)
    {
        if(mc->zreject)
        {
            // not deliverable
            remove_in_progress(qs->qfn);
            TRACK_ADD(QS_not_deliverable, 1);
        }
        else
            move_back_to_outbox(qs->qfn);
        qs->inprogress = 0;
        qs->fail();
        delete_later(qs);
        return;
    }
    qs->att_size = mc->expected_size;
    qs->do_store();
}


// note: once you call cancel, the state of the object
// is going to be unknown, and the object is going to be
// deleted soon. don't do anything to the object after
// calling cancel.
//
void
DwQSend::cancel()
{
    if(!inprogress)
    {
        delete_later(this);
        return;
    }

    cancel_op = 1;
    if(xfer_chan_id != -1)
    {
        MMChannel::synchronous_destroy(xfer_chan_id, MMChannel::HARD);
    }

    // note: there is a race here between the future
    // firing and the delete happening.
    // "cancel" is an operation that is a bit of an
    // abort, where i want minimal actions (including
    // cleanup). so there will be crumbs leftover after
    // this, which will be eventually cleaned up by
    // the "clean cruft" stuff.
    dirth_simulate_error_response(send_done_future);
    canceled();

    delete_later(this);
}


void
DwQSend::qd_set_status(MMChannel *mc, vc msg, void *, ValidPtr vp)
{
    mc->timer1.stop();
    mc->timer1.load(XFER_WATCHDOG_TIMEOUT);
    mc->timer1.start();
    if(!vp.is_valid())
        return;
    DwQSend *q = (DwQSend *)(void *)vp;
    if(q)
    {
        int e = mc->expected_size;
        if(e == 0)
            e = 1;
        int p = (int)(((double)mc->total_got * 100) / e);
        vc r = q->msg[QQM_RECIP_VEC][0];
        q->status_sig.emit(q->qfn, r, (const char *)msg, p);
    }
}

static
void
xfer_chan_call_succeeded(MMChannel *mc, int , vc, void *, ValidPtr)
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

void
DwQSend::xfer_chan_setup_timeout(MMChannel *mc, vc arg1, void *arg2, ValidPtr vp)
{
    DwQSend::eo_qd_xfer(mc, arg1, arg2, vp);
    mc->schedule_destroy(MMChannel::HARD);
    TRACK_ADD(QS_xfer_setup_timeout, 1);
}

int
DwQSend::send_with_attachment()
{
    vc sip;
    if(prev_ip.is_nil())
    {
        sip = get_random_xfer_server_ip(port);
        prev_ip = sip;
        prev_port = port;
        save_aux();
    }
    else
    {
        sip = prev_ip;
        port = prev_port;
    }

    ip = sip;
    // note: this is a trick. we send on the new xfer port because we
    // know we talk the new protocol. however, we leave the "port"
    // to the *old* protocol so old clients can still download the file.
    // newer clients will know to add the right amount to the port
    // that is stored with the message to fetch the message using newer protocols.
    vc send_port = port + vc(DWYCO_SEND_FILE_PORT_OFFSET); // new xfer protocol

    MMChannel *mc = 0;
    if(!sip.is_nil())
    {
        mc = new MMChannel;
        mc->tube = new DummyTube;
        mc->tube->connect((const char *)sip, 0, 0);
    }

    int i = -1;
    int state;
    if(sip.is_nil() || (state = mc->tube->gen_channel((int)send_port, i)) == SSERR)
    {
        move_back_to_outbox(qfn);
        inprogress = 0;
        return 0;
    }
    mc->remote_filename = att_basename;

    sproto *s = new sproto(i, file_send_server, mc->vp);
    // note: if a file already exists that has been encrypted, the key
    // is ignored, and it is assumed we have the encrypted msg
    // component with the right key in it (ie, we are resuming
    // an interrupted send.
    //s->file_key = key;

    mc->simple_protos[i] = s;
    s->start();

    mc->destroy_callback = eo_qd_xfer;
    mc->dcb_arg3 = vp;

    mc->status_callback = qd_set_status;
    mc->scb_arg1 = 0;
    mc->scb_arg2 = vp;
    mc->set_progress_status("Sending audio/video...");

    // start a short timer, since the builtin
    // connection timer in the tcp stack is
    // really too long
    //mc->timer1.set_autoreload(0);
    mc->timer1.set_oneshot(1);
    mc->timer1.load(CHANNEL_SETUP_TIMEOUT);
    mc->timer1.reset();
    mc->timer1.start();
    mc->timer1_callback = xfer_chan_setup_timeout;
    mc->t1cb_arg3 = vp;
    mc->chan_established_callback = xfer_chan_call_succeeded;

    mc->init_config(1);
    mc->recv_matches(mc->config);
    //mc->firewall_friendly = 0;

    mc->set_string_id("<<Background Zap Msg Send>>");
    xfer_chan_id = mc->myid;
    mc->start_service();
    return 1;
}

// returns -1 if the message to send is corrupt and can never
// be sent. in this case, the message given to the ctor is
// deleted and the caller should delete the object ASAP.
//
// returns 0 if the message is already in the process of
// being sent, or if the q operation is not performed.
// if 0 is returned, the send state machine is not started.
// in this case, the send can be retried (if the current
// operations on the message eventually fails, or if the
// send could not be started for a transient reason.)
//
// returns 1 if the message is queued successfully, and the
// send state machine is started to try and send it to the server.
//
int
DwQSend::send_message()
{
    if(cancel_op)
        return 0;

    {
        DwVecP<DwQSend> c = Qbm.query_by_member(qfn, &DwQSend::qfn);
        if(c.num_elems() > 1)
            return 0;
    }
    vc m;
    DwString afn = DwString("outbox" DIRSEPSTR "");
    afn += qfn;
    if(!load_info(m, afn.c_str()))
    {
        // corrupt or something
        Log->make_entry("deleting corrupt message");
        DeleteFile(newfn(afn).c_str());
        return -1;
    }
    if(!valid_qd_message(m))
    {
        Log->make_entry("bogus message deleted");
        DeleteFile(newfn(afn).c_str());
        return -1;
    }
    att_basename = m[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
    if(!att_basename.is_nil())
    {
        att_actual_fn = newfn(att_basename);
        if(access(att_actual_fn.c_str(), 0) != 0)
        {
            Log->make_entry("deleting corrupt message (no attachment)");
            DeleteFile(newfn(afn).c_str());
            return -1;
        }
        // att_actual_fn may get overwritten if the message is
        // encrypted. likewise with basename, but this is only
        // used to cleanup on success to remove the sent attachment
        alternate_att_actual_fn = att_actual_fn;
        has_att = 1;
    }
#if defined(ANDROID) || defined(DWYCO_IOS)
    // on mobile platforms, if there is a large number of sends
    // it can cause things to start getting sluggish with all the
    // encryption and whatnot, so we de-prioritize message sends
    // that include an attachment
    if(has_att)
    {
        DwVecP<DwQSend> c = Qbm.query_by_member(1, &DwQSend::has_att);
        if(c.num_elems() > 1)
            return 0;

    }

#endif

    DwString ipn("inprogress" DIRSEPSTR "");
    ipn += qfn;
    //DeleteFile(newfn(ipn).c_str());
    if(!move_replace(newfn(afn), newfn(ipn)))
        return 0;
    inprogress = 1;
    msg = m;
    dont_save_sent = msg[QQM_SAVE_SENT].is_nil();
    if(!load_aux())
    {
        start_time = (long)time(0);
        try_count = 1;
        save_aux();
    }
    else
    {
        try_count = (int)try_count + 1;
        save_aux();
        TRACK_ADD(QS_aux_found, 1);
    }
    vc pk;

    // see if there is an existing encrypted message, since we
    // can restart sends.
    DwString efn;
    efn += qfn;
    efn += ".enc";
    if(access(newfn(efn).c_str(), 0) == 0)
    {
        if(!load_info(emsg, efn.c_str()))
        {
            // corrupt or something
            Log->make_entry("deleting corrupt emsg");
            DeleteFile(newfn(efn).c_str());
            emsg = vcnil;
            TRACK_ADD(QS_restart_enc_bad_load, 1);
        }
        else
        {
            vc tmp_att_basename = emsg[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
            if(!tmp_att_basename.is_nil())
            {
                DwString tmp_att_actual_fn = newfn(tmp_att_basename);
                if(access(tmp_att_actual_fn.c_str(), 0) != 0)
                {
                    Log->make_entry("deleting corrupt enc message (no attachment)");
                    DeleteFile(newfn(efn).c_str());
                    emsg = vcnil;
                    TRACK_ADD(QS_restart_enc_bad_att, 1);
                }
                else
                {
                    att_actual_fn = tmp_att_actual_fn;
                    att_basename = tmp_att_basename;
                    TRACK_ADD(QS_restart_enc_att, 1);
                }
            }
            else
            {
                TRACK_ADD(QS_restart_enc_simple, 1);
            }
        }

    }
    if(emsg.is_nil())
    {
        vc recip_uid = m[QQM_RECIP_VEC][0];
        // fetch public keys each session, just in case.
        // note that if the key has changed for some reason,
        // this message may be dropped by the recipient because
        // of a key mismatch (the get_pk may return a stale key.
        // this case should be really rare.)
        // if the key hasn't been fetched before, the message
        // will be sent unencrypted until the key is fetched.
        // it probably makes sense to try and fetch keys that are
        // likely to be used, but that complicates things. for now,
        // keep it simple.
        if(!pk_session_cached(recip_uid))
        {
            fetch_info(recip_uid);
            pk_set_session_cache(recip_uid);
        }

        vc alt_pk;
        vc alt_name;

        if(!Avoid_pk && get_pk2(recip_uid, pk, alt_pk, alt_name))
        {
            vc pkeys(VC_VECTOR);
            pkeys[0] = pk;
            // note: this was just for testing
            //pkeys[1] = Current_alternate->my_static_public();
            if(!alt_pk.is_nil())
            {
                pkeys[1] = alt_pk;
            }
            dhsf = dh_store_and_forward_material2(pkeys, key);
            if(dhsf.is_nil())
            {
                // this is one of those queasy cases where there is something
                // wrong with the public key we got locally. it could be because
                // of a million different innocuous reasons (viruses, virus checkers,
                // disk corruption, etc.)
                // under the circumstances, we should just remove the pk (and hopefully
                // get another copy from the server eventually) and send the message
                // unencrypted
                emsg = vcnil;
                pk_invalidate(recip_uid);
                TRACK_ADD(QS_pk_problem, 1);
            }
            else
            {
                ectx = vclh_encdec_open();
                vclh_encdec_init_key_ctx(ectx, key, 0);
                // encrypt the entire msg and store it so we can restart if
                // we end up getting interrupted
                emsg = encrypt_msg_qqm(msg, dhsf, ectx, key);

                if(!save_info(emsg, efn.c_str()))
                {
                    emsg = vcnil;
                    // default back to sending unencrypted (note: channel is still
                    // encrypted, just sitting on server will not be.)
                    TRACK_ADD(QS_enc_failed, 1);
                }
                else
                {
                    att_basename = emsg[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
                    if(!att_basename.is_nil())
                        att_actual_fn = newfn(att_basename);
                    TRACK_ADD(QS_enc_ok, 1);
                }

                // at this point, if emsg is non-nil, we have both the
                // *.q msg with its attachment, and the *.q.enc and an encrypted attachment.dyc
                // saved in the filesystem.
            }
        }
        else
        {
            if(!Avoid_pk)
                TRACK_ADD(QS_enc_pk_not_available, 1);
        }

    }
    if((Force_pk || force_encryption) && emsg.is_nil())
    {
        // just defer the send, with any luck we have sent the
        // request for the key and the server will have responded
        // by the time we get back here.
        return 0;
    }
    if(att_basename.is_nil())
    {
        start();
        do_store();
    }
    else
    {
        if(send_with_attachment())
            start();
        else
            return 0;
    }
    return 1;
}
}

