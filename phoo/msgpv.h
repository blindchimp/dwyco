
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

int preview_saved_msg(const QByteArray& mid, QByteArray& preview_fn, int& file, QByteArray &full_size_filename, QString& local_time);
int preview_msg_body(DWYCO_SAVED_MSG_LIST sm, QByteArray& preview_fn, int& file, QByteArray &full_size_filename, QString& local_time);
void init_supported_image_exts();

// returns 1 if the extension of "user_fn" is one of the image types we
// know how to make a preview for. this is the same test preview_saved_msg
// uses, so the ui can tell "this is a picture" from "this is some other
// kind of file" without guessing.
int msg_file_is_image(const QByteArray& user_fn);

// fills in the original (sender-supplied) filename and the size in bytes of
// the local copy of a "file" attachment. returns 1 if the message has a
// user file attachment at all. is_image is set to 1 if we know how to make
// a preview for it, and 0 otherwise. on failure, user_fn is empty, size is
// 0 and is_image is 0.
int msg_file_info(const QByteArray& mid, QByteArray& user_fn, qint64& size, int& is_image);
int msg_body_file_info(DWYCO_SAVED_MSG_LIST qsm, QByteArray& user_fn, qint64& size, int& is_image);
#endif
