
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AEXTSDL_H
#define AEXTSDL_H

void audout_sdl_new(void *);
void audout_sdl_delete(void *);
int audout_sdl_init(void *ha);
int audout_sdl_device_output(void *, void *buf, int len, int user_data);
int audout_sdl_device_done(void *, void **buf_out, int *len, int *user_data);
int audout_sdl_device_stop(void *);
int audout_sdl_device_reset(void *);
int audout_sdl_device_status(void *);
int audout_sdl_device_close(void *ah);
int audout_sdl_device_buffer_time(void *, int sz);
int audout_sdl_device_play_silence(void *ah);
int audout_sdl_device_bufs_playing(void *ah);
#endif
