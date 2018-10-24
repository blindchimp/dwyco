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

int dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, int& zap_viewer, QByteArray& mid_out, int &has_att, int &is_file);
#endif
