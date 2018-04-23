
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef GETINFO_H
#include <QString>
#include <QByteArray>

QString dwyco_info_to_display(const QByteArray& uid);
QString dwyco_info_to_display2(const QByteArray& uid, const char *field);
void get_review_status(const QByteArray& uid, int& reviewed, int& regular);
#endif
