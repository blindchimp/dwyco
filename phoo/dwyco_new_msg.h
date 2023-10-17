
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef dwyco_new_msg_h
#define dwyco_new_msg_h
#include <QByteArray>
#include "dlli.h"

//int dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, QByteArray& mid_out);
int dwyco_process_unfetched_list(DWYCO_UNFETCHED_MSG_LIST ml, QSet<QByteArray> &uids);
void add_unviewed(const QByteArray& uid, const QByteArray& mid);
void del_unviewed_uid(const QByteArray& uid);
void del_unviewed_mid(const QByteArray& uid, const QByteArray& mid);
void del_unviewed_mid(const QByteArray& mid);
bool uid_has_unviewed_msgs(const QByteArray &uid);
//int uid_unviewed_msgs_count(const QByteArray &uid);
bool any_unviewed_msgs();
void load_unviewed();
bool got_msg_this_session(const QByteArray &uid);
void clear_session_msg();
void clear_unviewed_msgs();
void load_inbox_tags_to_unviewed(QSet<QByteArray>&);
void add_got_msg_from(const QByteArray& uid);
QList<QByteArray> uids_with_unviewed();

#endif
