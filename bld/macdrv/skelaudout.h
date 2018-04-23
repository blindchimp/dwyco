
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SKELAUDOUT_H
#define SKELAUDOUT_H

void skel_audout_new(void *);
void skel_audout_delete(void *);
int skel_audout_init(void *ha);
int skel_audout_device_output(void *, void *buf, int len, int user_data);
int skel_audout_device_done(void *, void **buf_out, int *len, int *user_data);
int skel_audout_device_stop(void *);
int skel_audout_device_reset(void *);
int skel_audout_device_status(void *);
int skel_audout_device_close(void *ah);
int skel_audout_device_buffer_time(void *, int sz);
int skel_audout_device_play_silence(void *ah);
int skel_audout_device_bufs_playing(void *ah);
#endif
