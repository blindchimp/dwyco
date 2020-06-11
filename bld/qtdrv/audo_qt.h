
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AUDO_QT_H
#define AUDO_QT_H

#include "dlli.h"

void DWYCOCALLCONV audout_qt_new(void *);
void DWYCOCALLCONV audout_qt_delete(void *);
int DWYCOCALLCONV audout_qt_init(void *ha);
int DWYCOCALLCONV audout_qt_device_output(void *, void *buf, int len, int user_data);
int DWYCOCALLCONV audout_qt_device_done(void *, void **buf_out, int *len, int *user_data);
int DWYCOCALLCONV audout_qt_device_stop(void *);
int DWYCOCALLCONV audout_qt_device_reset(void *);
int DWYCOCALLCONV audout_qt_device_status(void *);
int DWYCOCALLCONV audout_qt_device_close(void *ah);
int DWYCOCALLCONV audout_qt_device_buffer_time(void *, int sz);
int DWYCOCALLCONV audout_qt_device_play_silence(void *ah);
int DWYCOCALLCONV audout_qt_device_bufs_playing(void *ah);
#endif
