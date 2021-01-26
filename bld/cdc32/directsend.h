
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
#include "qdirth.h"
#include "qmsg.h"
#include "filetube.h"
#include "se.h"

class MMCall;

namespace dwyco {

class DirectSend : public ssns::trackable
{
    DirectSend(const DirectSend&) = delete;
    DirectSend& operator=(const DirectSend&) = delete;

    // just a note: don't try to make this private unless you want a lot
    // of grief. there are a lot of non-member functions that need access to this
    // and i don't feel like calling them out as friends or statics in here.
    //
public:

    DirectSend(const DwString& fn);
    ~DirectSend();

    static DwQueryByMember<DirectSend> Qbm;

    ValidPtr vp;

    int inprogress;
    int cancel_op;
    DwString qfn;
    DwString local_mid;
    vc msg_to_send;
    DwString actual_filename;
    vc file_basename;
    vc msg;
    vc small_attachment;
    vc uid;

    // status is first arg
    // second arg is persistent id, ie, the name of the .q file
    // third arg is recipient uid
    ssns::signal3<enum dwyco_sys_event, DwString, vc> se_sig;

    // transfer statuses, useful for debugging
    // (pers-id, ruid, msg, percent)
    ssns::signal4<DwString, vc, DwString, int> status_sig;

    void send_direct();
    void send_with_attachment();
    int send_message();
    int load_small_attachment();
    void start_send();

    void cancel();

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
