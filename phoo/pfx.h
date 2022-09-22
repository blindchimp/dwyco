
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PFX_H
#define PFX_H
#include <QByteArray>
#include <QString>

extern QByteArray User_pfx;
extern QByteArray Sys_pfx;
extern QByteArray Tmp_pfx;

extern QByteArray User_pfx_native;
extern QByteArray Sys_pfx_native;
extern QByteArray Tmp_pfx_native;

QByteArray add_pfx(const QByteArray& pfx, const QByteArray& fn);
QByteArray random_fn();
QByteArray get_extended(QByteArray txt);
#endif
