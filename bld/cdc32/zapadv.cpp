
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "zapadv.h"
#include "doinit.h"
#include "qmsg.h"
#include "qdirth.h"

ZapAdvXfer ZapAdvData;

#define ZAP_SECTION "zap"
#define ALWAYS_SERVER "always_server"
#define ALWAYS_ACCEPT "global_accept"
#define USE_OLD_TIMING "old_timing"
//#define ONLY_PALS "only_pals"
#define NO_FORWARD "no_forward"
#define AUTO_REPLY_MID "auto_reply_mid"
#define SEND_AUTO_REPLY "send_auto_reply"

#define DEFAULT_ALWAYS_SERVER 0
#define DEFAULT_ALWAYS_ACCEPT 0
#define DEFAULT_USE_OLD_TIMING 0
//#define DEFAULT_ONLY_PALS 0
#define DEFAULT_NO_FORWARD_DEFAULT 0
#define DEFAULT_SAVE_SENT 1
#define DEFAULT_AUTO_REPLY_MID ""
#define DEFAULT_SEND_AUTO_REPLY 0

#define OTHERZAPS "otherzaps"
#define DEFAULT_OTHERZAPS OZ_RECV_ALL
#define OZ_IGNORE 0
#define OZ_ZSAVE 1
#define OZ_RECV_ALL 2

ZapAdvXfer::ZapAdvXfer()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(always_server),
DWUIINIT_CTOR_VAL(ignore),
DWUIINIT_CTOR_VAL(recv_all),
DWUIINIT_CTOR_VAL(zsave),
DWUIINIT_CTOR_VAL(always_accept),
DWUIINIT_CTOR_VAL(use_old_timing),
DWUIINIT_CTOR_VAL(save_sent),
//DWUIINIT_CTOR_VAL(only_pals),
DWUIINIT_CTOR_VAL(no_forward_default),
DWUIINIT_CTOR_VAL(auto_reply_mid),
DWUIINIT_CTOR_VAL(send_auto_reply)
DWUIINIT_CTOR_END
{
    set_always_server(DEFAULT_ALWAYS_SERVER);
    set_ignore(0);
    set_recv_all(1);
    set_zsave(0);
    set_always_accept(DEFAULT_ALWAYS_ACCEPT);
    set_use_old_timing(DEFAULT_USE_OLD_TIMING);
    set_no_forward_default(DEFAULT_NO_FORWARD_DEFAULT);
    set_save_sent(DEFAULT_SAVE_SENT);
    //set_only_pals(DEFAULT_ONLY_PALS);
    set_auto_reply_mid(DEFAULT_AUTO_REPLY_MID);
    set_send_auto_reply(DEFAULT_SEND_AUTO_REPLY);
}

void
ZapAdvXfer::save()
{
    save_syncmap(syncmap, ZAP_SECTION ".dif");
}

void
ZapAdvXfer::load()
{
    load_syncmap(syncmap, ZAP_SECTION ".dif");
}

