
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef dwyco_new_msg_h
#define dwyco_new_msg_h
#include <QSet>
#include "dwstr.h"


int dwyco_new_msg(DwOString& uid_out, DwOString& txt, int& zap_viewer, DwOString& mid_out);
void add_unviewed(const QByteArray& uid, const QByteArray& mid, int no_save = 0);
void del_unviewed_uid(const QByteArray& uid);
void del_unviewed_mid(const QByteArray& uid, const QByteArray& mid);
void del_unviewed_mid(const QByteArray& mid);
int uid_has_unviewed_msgs(const QByteArray &uid);
int uid_unviewed_msgs_count(const QByteArray &uid);
int has_unviewed_msgs();
int load_unviewed();
int reload_msgs();
int session_msg(const QByteArray &uid);
int any_unread_msg(const QByteArray& uid);
#endif
