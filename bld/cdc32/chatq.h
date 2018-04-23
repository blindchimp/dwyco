
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CHATQ_H
#define CHATQ_H
#include "vc.h"

int chatq_send_addq(int q, vc uid);
int chatq_send_delq(int q, vc uid);
int chatq_send_data(int q, vc uid, const char *buf, int len);
int chatq_send_talk(int q, vc uid);
int chatq_send_mute(int q, vc uid, vc on);
int chatq_send_pals_only(int q, vc uid, vc on);
int chatq_send_set_chat_filter(int q, vc only_gods, vc only_demi_gods, vc only_uid);
int chatq_send_set_demigod(vc uid, vc on);
int chatq_send_clear_all_demigods();
int chatq_send_set_unblock_time(int q, vc uid, int tm, vc reason);
int chatq_send_get_admin_info();
int chatq_send_set_sys_attr(vc name, vc val);
int chatq_send_update_call_accept();
int chatq_send_update_pals(vc pals);
int chatq_send_activity_state(vc state);
int chatq_send_popup(vc text, int global);
int chatq_send_chat(vc chan, vc chat, int pic_type, vc pic_data);
int chatq_send_selective_chat(vc chan, vc recipients, vc msg);

#endif
