
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

class MMCall;
class MMChannel;

namespace dwyco {

// this class sends a message over a "direct" link between two
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
// wierd special cases:
//
// if you send a message with an attachment to yourself, the files
// are simply shuffled around locally, and corresponding start/success
// signals are emitted immediately from the "send_message" call.
//
// (the following is a deficiency, and needs to be fixed.)
// if you are sending a message with an attachment, and the link that
// is eventually setup is via a server proxy, this class fails immediately
// (presumably so you can send it via server.) if the link is direct
// the send occurs normally.
// note: attachments that are considered "small" (about 20k) are sent
// inline, and that works both direct and via proxy.
//

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
    ssns::signal3<enum dwyco_sys_event, const DwString&, vc> se_sig;

    // transfer statuses, useful for debugging
    // (pers-id, ruid, msg, percent)
    ssns::signal4<const DwString&, vc, const DwString&, int> status_sig;

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
