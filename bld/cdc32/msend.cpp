
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "directsend.h"
#include "qsend.h"
#include "dlli.h"
#include "se.h"
#include "msend.h"
#include "qmsg.h"
#include "xinfo.h"
#include "ezset.h"

namespace dwyco {

static
void
qs_signal_bounce(enum dwyco_sys_event status, const DwString& qfn, vc ruid)
{
    // avoid multiple start messages for one "round" of send attempts
    if(status == SE_MSG_SEND_START)
        return;
    // by default, it gets put back in the outbox for a retry.
    // the caller can cancel and remove the message if they want
    se_emit_msg(status, qfn, ruid);

}

static
int
send_via_server_int(const DwString& qfn, int inhibit_encryption, int no_group, int no_self)
{
    // assumes already in the outbox

    DwQSend *qs = new DwQSend(qfn, 0);
    qs->se_sig.connect_ptrfun(qs_signal_bounce);
    qs->status_sig.connect_ptrfun(se_emit_msg_status);
    qs->force_encryption = inhibit_encryption ? DwQSend::INHIBIT_ENCRYPTION : DwQSend::DEFAULT;
    qs->no_group = no_group;
    qs->no_self_send = no_self;
    int err;
    err = qs->send_message();
    if(err == -1)
    {
        // at this point, the message can never be sent
        // because something is corrupted
        se_emit_msg(SE_MSG_SEND_FAIL, qfn, vcnil);
        delete qs;
        return 0;
    }
    else if(err == 0)
    {
        // some transient error, but it is q'd
        move_back_to_outbox(qfn);
        se_emit_msg(SE_MSG_SEND_FAIL, qfn, vcnil);
        delete qs;
    }
    return 1;
}

static
void
ds_signal_bounce(enum dwyco_sys_event status, const DwString& qfn, vc ruid)
{
    if(status == SE_MSG_SEND_SUCCESS ||
            status == SE_MSG_SEND_START ||
            status == SE_MSG_SEND_DELIVERY_SUCCESS ||
            status == SE_MSG_SEND_CANCELED)
    {
        se_emit_msg(status, qfn, ruid);
        return;
    }

    // if it is a fail, try sending it via server
    No_direct_msgs.add(ruid);
    move_back_to_outbox(qfn);
    send_via_server_int(qfn, 0, 0, 0);
}

int
send_via_server(const DwString& qfn, int inhibit_encryption, int no_group, int no_self)
{
    move_to_outbox(qfn);

    return send_via_server_int(qfn, inhibit_encryption, no_group, no_self);
}

int
send_via_server_deferred(const DwString& qfn)
{
    move_to_outbox(qfn);

    DwQSend *qs = new DwQSend(qfn, 1);
    qs->se_sig.connect_ptrfun(qs_signal_bounce);
    qs->status_sig.connect_ptrfun(se_emit_msg_status);
    int err;
    err = qs->send_message();
    if(err == -1)
    {
        // at this point, the message can never be sent
        // because something is corrupted
        se_emit_msg(SE_MSG_SEND_FAIL, qfn, vcnil);
        delete qs;
        return 0;
    }
    else if(err == 0)
    {
        // some transient error, but it is q'd
        move_back_to_outbox(qfn);
        se_emit_msg(SE_MSG_SEND_FAIL, qfn, vcnil);
        delete qs;
    }
    return 1;
}

static
int
has_attachment(const DwString& qfn)
{
    vc msg_to_send;
    if(!load_info(msg_to_send, qfn.c_str()))
    {
        return 0;
    }
    if(!msg_to_send[QQM_MSG_VEC][QQM_BODY_ATTACHMENT].is_nil())
    {
        return 1;
    }
    return 0;
}


int
send_best_way(const DwString& qfn, vc ruid)
{
    if(No_direct_msgs.contains(ruid) ||
            (No_direct_att.contains(ruid) && has_attachment(qfn))
            || (int)get_settings_value("zap/always_server") == 1)
        return send_via_server(qfn);

    // basic idea is to try and send directly, which may involve some setup
    // delays while we get a channel set up to the recipient.
    // if there is some problem there, we switch to server send.

    // some considerations: if we are in the middle of a direct send process, and
    // we get shut down (like on android), we want to be able to resume (though
    // maybe just through the server.) so we need a place to q up the message
    // that will cause it not to be picked up while we are operating on it, but
    // that will eventually be picked up and sent.

    DirectSend *ds = new DirectSend(qfn);
    ds->se_sig.connect_ptrfun(ds_signal_bounce);
    ds->status_sig.connect_ptrfun(se_emit_msg_status);
    int dsres = ds->send_message();
    if(dsres == 0)
    {
        if(ds->inprogress)
        {
            move_back_to_outbox(qfn);
        }
        delete ds;
        return send_via_server_int(qfn, 0, 0, 0);
    }
    else if(dsres == -1)
    {
        // msg is corrupt or something
        delete ds;
        return 0;
    }

    return 1;
}

// arrange to get a message send terminated if it exists, and remove the
// message as well so it is never sent.
static
void
delete_message(enum dwyco_sys_event, const DwString& qfn, vc)
{
    // note: on windows, we may not be able to delete a message if it is
    // still open. this is likely to happen more with attachments, so
    // we don't worry about it too much and just let the post-run cleanup
    // thing find and delete orphaned attachments.
    remove_from_outbox(qfn);
    remove_in_progress(qfn);

}

int
kill_message(const DwString& qfn)
{
    // get rid of it immediately if possible (we don't generally keep the
    // .q file open, so it can be almost always deleted, even on windows)
    delete_message(SE_NOTHING, qfn, vcnil);

    // note: for direct send cancels, any in-progress attachment send is synchronously
    // stopped, and any unanswered send callbacks are canceled. the message
    // should be obliterated. the cancel signal is emitted which causes
    // progression to another send step to stop
    DwVecP<DirectSend> dsv = DirectSend::Qbm.query_by_member(qfn, &DirectSend::qfn);
    for(int i = 0; i < dsv.num_elems(); ++i)
    {
        DirectSend *ds = dsv[i];
        // connect cancel signal to function that will destroy the message if
        // needed. this is to make sure that clients will get all their signals
        // properly before we delete the message
        ds->se_sig.connect_ptrfun(delete_message);
        ds->cancel();

    }

    // for server cancel,
    DwVecP<DwQSend> dsq = DwQSend::Qbm.query_by_member(qfn, &DwQSend::qfn);
    for(int i = 0; i < dsq.num_elems(); ++i)
    {
        DwQSend *ds = dsq[i];
        // connect cancel signal to function that will destroy the message if
        // needed. this is to make sure that clients will get all their signals
        // properly before we delete the message
        ds->se_sig.connect_ptrfun(delete_message);
        ds->cancel();

    }
    return 1;

}

}
