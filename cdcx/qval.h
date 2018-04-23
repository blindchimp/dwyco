
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QVAL_H
#define QVAL_H
#include <QValidator>
QValidator * email_validator();
QValidator * size_validator();
QValidator * subcode_validator();

#endif
