
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGPV_H
#define MSGPV_H
#include <QByteArray>
#include <QString>
#include "dlli.h"

int preview_saved_msg(const QByteArray& uid, const QByteArray& mid, QByteArray& preview_fn, int& file, QByteArray &full_size_filename, QString& local_time);
int preview_unsaved_msg(DWYCO_UNSAVED_MSG_LIST sm, QByteArray& preview_fn, int& file, QByteArray &full_size_filename, QString& local_time);
#endif
