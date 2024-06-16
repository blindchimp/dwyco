
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QSEND_H
#define QSEND_H
#include "dwstr.h"
#include "vc.h"
#include "pval.h"
#include "ssns.h"
#include "qdirth.h"
#include "dwqbm.h"
#include "se.h"
class MMChannel;

// send a message via the server with optional public key encryption.
// stores the message so it can be re-tried automatically if the
// send is not successful.
// the "qfn" file that is required by this class is created using
// the q_message functions.
namespace dwyco {
class DwQSend
{
    DwQSend(const DwQSend&) = delete;
    DwQSend& operator=(const DwQSend&) = delete;

    static void qd_send_done(vc m, void *, vc, ValidPtr vp);
    static void eo_qd_xfer(MMChannel *mc, vc, void *, ValidPtr vp);
    static void qd_set_status(MMChannel *mc, vc msg, void *, ValidPtr vp);
    static void xfer_chan_setup_timeout(MMChannel *mc, vc arg1, void *arg2, ValidPtr vp);
    static void delete_later(DwQSend *d);

    ValidPtr vp;

public:
    DwQSend(const DwString& qfn, int defer_send);
    ~DwQSend();

    DwString qfn;
    static dwyco::DwQueryByMember<DwQSend> Qbm;
    int inprogress;

    enum enc_mode {
        // use PK encryption if we have recipient's public key locally.
        // otherwise, send the message as is, and initiate a fetch to
        // get the public key.
        DEFAULT = 0,
        // use PK encryption, but if we don't have the recipient's public key locally
        // just defer sending the message, and initiate a fetch to get
        // the public key. the caller will have to retry the send at a later time.
        FORCE_ENCRYPTION = 1,
        // don't try to use PK encryption, the message is sent as is.
        INHIBIT_ENCRYPTION = 2
    };
    enum enc_mode force_encryption;
    int no_self_send;
    int no_group;

    int send_message();
    void cancel();
    // for now, same as MessageSendCallback
    ssns::signal6<int, int, const char *, int, const char *, void *> sig;
    void *mscb_arg1;

    // status is first arg
    // second arg is persistent id, ie, the name of the .q file
    // third arg is recipient uid
    ssns::signal3<enum dwyco_sys_event, const DwString&, const vc&> se_sig;

    // transfer statuses, useful for debugging
    // (pers-id, ruid, msg, percent)
    ssns::signal4<const DwString&, const vc&, const DwString&, int> status_sig;

private:

    int cancel_op;
    int defer_send;

    vc msg;
    vc emsg;
    vc dhsf;
    vc key;
    vc ectx;
    vc att_basename;
    // note: when msg is encrypted, both the plain
    // and encrypted attachments exist until the message
    // is completely sent.
    DwString att_actual_fn;
    DwString alternate_att_actual_fn;

    int has_att;
    // this creates a subtle problem if we name messages locally
    // from the server that end up on the recipient with a slightly
    // different set of data (ie, it will be flagged "sent" here, and
    // "received" at the recipient). which causes a situation where
    // the group stuff gets confused because it is assuming messages
    // have unique mid's. since we were only doing this because we thought
    // we could do "delivery reports", and it didn't work out too well,
    // we will just create new random mid's again like in the past.
    //vc delivered_mid;
    int dont_save_sent;
    int att_size;
    QckDone send_done_future;
    vc ip;
    vc port;

    // auxilliary info we keep between tries, mainly to resume
    // partial transfers, but also we can indicate when the
    // and how the message should be disposed of if it can't be sent at
    // all.
    vc start_time;
    vc try_count;
    vc prev_ip;
    vc prev_port;
    void save_aux();
    int load_aux();
    void remove_aux();

    int xfer_chan_id;

    void do_store();
    int send_with_attachment();

    // these emit signals as the message send
    // state machine proceeds
    void fail();
    void succeed();
    void start();
    void canceled();
};

extern int Avoid_pk;
extern int Force_pk;

}

#endif
