
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include "skelaudout.h"

void DWYCOCALLCONV      mac_audio_new(void *, int buflen, int nbufs);
void DWYCOCALLCONV      mac_audio_delete(void *);
int DWYCOCALLCONV       mac_audio_init(void *);
int DWYCOCALLCONV       mac_audio_has_data(void *);
void DWYCOCALLCONV      mac_audio_need(void *);
void DWYCOCALLCONV      mac_audio_pass(void *);
void DWYCOCALLCONV      mac_audio_stop(void *);
void DWYCOCALLCONV      mac_audio_on(void *);
void DWYCOCALLCONV      mac_audio_off(void *);
void DWYCOCALLCONV      mac_audio_reset(void *);
int DWYCOCALLCONV       mac_audio_status(void *);
void * DWYCOCALLCONV mac_audio_get_data(void *, int *r, int *c);

void DWYCOEXPORT	mac_vg_set_appdata(void *u1);
void DWYCOEXPORT	mac_vgnew(void *aqext);
void DWYCOEXPORT	mac_vgdel(void *aqext);
int DWYCOEXPORT		mac_vginit(void *aqext, int frame_rate);
int DWYCOEXPORT		mac_vghas_data(void *aqext);
void DWYCOEXPORT	mac_vgneed(void *aqext);

void DWYCOEXPORT	mac_vgpass(void *aqext);

void DWYCOEXPORT	mac_vgstop(void *aqext);
void * DWYCOEXPORT	mac_vgget_data(void *aqext,
                                   int *c_out,
                                   int *r_out,
                                   int *bytes_out,
                                   int *fmt_out,
                                   unsigned long *captime_out);

void DWYCOEXPORT	mac_vgfree_data(void *data);

char ** DWYCOEXPORT	mac_vgget_video_devices();
void DWYCOEXPORT	mac_vgfree_video_devices(char **);
void DWYCOEXPORT	mac_vgset_video_device(int idx);
void DWYCOEXPORT	mac_vgstop_video_device();
void DWYCOEXPORT	mac_vgshow_source_dialog();
void DWYCOEXPORT	mac_vgpreview_on(void *display_window);
void DWYCOEXPORT	mac_vgpreview_off();
void DWYCOEXPORT	mac_vg_set_appdata(void *u1);

void
init_mac_drivers()
{
    dwyco_set_external_audio_capture_callbacks(
        mac_audio_new,
        mac_audio_delete,
        mac_audio_init,
        mac_audio_has_data,
        mac_audio_need,
        mac_audio_pass,
        mac_audio_stop,
        mac_audio_on,
        mac_audio_off,
        mac_audio_reset,
        mac_audio_status,
        mac_audio_get_data);

    dwyco_set_external_audio_output_callbacks(
        skel_audout_new,
        skel_audout_delete,
        skel_audout_init,
        skel_audout_device_output,
        skel_audout_device_done,
        skel_audout_device_stop,
        skel_audout_device_reset,
        skel_audout_device_status,
        skel_audout_device_close,
        skel_audout_device_buffer_time,
        skel_audout_device_play_silence,
        skel_audout_device_bufs_playing
    );

    dwyco_set_external_video_capture_callbacks(
        mac_vgnew,
        mac_vgdel,
        mac_vginit,
        mac_vghas_data,
        mac_vgneed,
        mac_vgpass,
        mac_vgstop,
        mac_vgget_data,
        mac_vgfree_data,
        mac_vgget_video_devices,
        mac_vgfree_video_devices,
        mac_vgset_video_device,
        mac_vgstop_video_device,
        mac_vgshow_source_dialog,
        mac_vgpreview_on,
        mac_vgpreview_off,
        mac_vg_set_appdata
    );


}
