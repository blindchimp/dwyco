
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DIRECTSEND_H
#define DIRECTSEND_H
#include "pval.h"
#include "vc.h"
#include "dwstr.h"
#include "ssns.h"
#include "dwqbm.h"
#include "se.h"

class MMChannel;

namespace dwyco {
class MMCall;
// DirectSend sends a message over a "direct" link between two
// clients. there is no encryption done here, the link that is used
// is an encrypted tunnel. if a suitable link does not exist
// to the recipient (this only sends to one recipient), a link setup is
// requested, and when the link is established, the send operation is
// started.
//
// the message is stored in the inprogress box while it is being sent
// to the remote side. if there is a failure detected, the message is
// moved to the outbox. the client can decide what to do with the
// message at that point. this class does not attempt a retry.
//
// a note on DwQSend life-time:
// when you create this object with a queued message filename, then call
// send_message, it takes care of deleting itself. the only time you should
// manually delete this object is if you create one, then decide immediately
// to get rid of it without calling send_message or cancel.
//
// wierd special cases:
//
// if you send a message with an attachment to yourself, the files
// are simply shuffled around locally, and corresponding start/success
// signals are emitted immediately from the "send_message" call.
//
// Large attachments are sent via a secondary reliable channel.
// For direct connections this is a direct TCP channel to the peer's
// listening port; for proxy connections an "aux_r" is sent on the
// control channel to rendezvous a second channel via the relay
// (same mechanism as media aux channels in mmchan.cc).
// Small attachments (<= Inline_attach_size) are sent inline via
// the control channel and work over either transport.
//
// note: because of a design deficiency in the file transfer stuff,
// we have to limit ourselves to one transfer at a time. if a second
// transfer is attempted, it is "failed", assuming the caller will
// queue the message to the server. this is unfortunate because i think
// the "no direct attachment" filter gets set in this case, causing all
// subsequent messages to go via server. this should really be remedied.

class DirectSend : public ssns::trackable
{
    DirectSend(const DirectSend&) = delete;
    DirectSend& operator=(const DirectSend&) = delete;

    static void delete_later(DirectSend *d);
    static void send_done(vc m, void *, vc, ValidPtr vp);
    static void eo_direct_xfer(MMChannel *mc, vc, void *, ValidPtr vp);
    static void set_status(MMChannel *mc, vc msg, void *, ValidPtr vp);
    static void xfer_chan_setup_timeout(MMChannel *mc, vc arg1, void *arg2, ValidPtr vp);

    ValidPtr vp;

public:

    DirectSend(const DwString& fn);
    ~DirectSend();

    static DwQueryByMember<DirectSend> Qbm;
    int send_message();
    void cancel();
    int inprogress;
    DwString qfn;
    static int Inline_attach_size;

    // status is first arg
    // second arg is persistent id, ie, the name of the .q file
    // third arg is recipient uid
    ssns::signal3<enum dwyco_sys_event, const DwString&, const vc&> se_sig;

    // transfer statuses, useful for debugging
    // (pers-id, ruid, msg, percent)
    ssns::signal4<const DwString&, const vc&, const DwString&, int> status_sig;

private:

    int cancel_op;

    DwString local_mid;
    vc msg_to_send;
    DwString actual_filename;
    vc file_basename;
    vc msg;
    vc small_attachment;
    vc uid;

    void send_direct();
    void send_with_attachment();

    int load_small_attachment();
    void start_send();

    void fail();
    void succeed();
    void start();
    void canceled();

    void call_disposition(MMCall *, int, void *, ValidPtr);
    void cleanup_after_send();
    int dont_save_sent;
    MMChannel *xfer_channel;
    int xfer_chan_id;
    int send_chan_id;
};

}


#endif // DIRECTSEND_H
