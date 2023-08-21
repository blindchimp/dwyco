
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VGQT_H
#define VGQT_H
#include "dlli.h"

void DWYCOEXPORT vgqt_new(void *aqext);
void DWYCOEXPORT vgqt_del(void *aqext);
int DWYCOEXPORT vgqt_init(void *aqext, int frame_rate);
int DWYCOEXPORT vgqt_has_data(void *aqext);
void DWYCOEXPORT vgqt_need(void *aqext);
void DWYCOEXPORT vgqt_pass(void *aqext);
void DWYCOEXPORT vgqt_stop(void *aqext);
void * DWYCOEXPORT vgqt_get_data(
    void *aqext,
    int *c_out, int *r_out,
    int *bytes_out, int *fmt_out, unsigned long *captime_out);

void DWYCOEXPORT vgqt_free_data(void *data);
char **DWYCOEXPORT vgqt_get_video_devices();
void DWYCOEXPORT vgqt_free_video_devices(char **);
void DWYCOEXPORT vgqt_set_video_device(int idx);
void DWYCOEXPORT vgqt_stop_video_device();
#endif
