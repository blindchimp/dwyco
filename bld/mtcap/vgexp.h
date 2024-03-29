#ifndef VGEXP_H
#define VGEXP_H

#include "dlli.h"

//#define DWYCOCALLCONV __stdcall
//#define DWYCOEXPORT __declspec(dllexport) DWYCOCALLCONV
// this must be called first with the TApplication
extern "C" {
void DWYCOEXPORT vg_set_appdata(void *u1);

void DWYCOEXPORT vgnew(void *aqext);
void DWYCOEXPORT vgdel(void *aqext);
int DWYCOEXPORT vginit(void *aqext, int frame_rate);
int DWYCOEXPORT vghas_data(void *aqext);
void DWYCOEXPORT vgneed(void *aqext);

void DWYCOEXPORT vgpass(void *aqext);

void DWYCOEXPORT vgstop(void *aqext);
void * DWYCOEXPORT vgget_data(
void *aqext,
	int *c_out, int *r_out,
	int *bytes_out, int *fmt_out, unsigned long *captime_out);

void DWYCOEXPORT vgfree_data(void *data);

char **DWYCOEXPORT vgget_video_devices();
void DWYCOEXPORT vgfree_video_devices(char **);
void DWYCOEXPORT vgset_video_device(int idx);
void DWYCOEXPORT vgstop_video_device();
void DWYCOEXPORT vgshow_source_dialog();
// these are needed so we can show video preview
// while the source dialog is up and adjusting
void DWYCOEXPORT vgpreview_on(void *display_window);
void DWYCOEXPORT vgpreview_off();
}

#endif
