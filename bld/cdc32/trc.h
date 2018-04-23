
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TRC_H
#define TRC_H
#ifdef DWYCO_TRACE
void
hook_DwycoChannelDestroyCallback(DwycoChannelDestroyCallback& user_cb, void *&user_arg);
#endif
#endif
