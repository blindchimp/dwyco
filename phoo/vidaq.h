
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VIDAQ_H
#define VIDAQ_H
// abstract video aquisition object
// $Header: g:/dwight/repo/cdc32/rcs/vidaq.h 1.9 1999/01/10 16:10:55 dwight Checkpoint $
#include <string.h>

class VidAquire
{
public:
	static void *dummy;
	static int dummy2;
	static unsigned long dummy3;

public:

	VidAquire() {inited = 0;}
	virtual ~VidAquire() {}

   int init_ok() {return inited;}

	// can't unload frame now, buffer it, or toss it
   // if necessary (like not enough memory.)
	virtual void pass() = 0;

	// can accept more data (fires up aquisition if
	// it has been halted.)
	virtual void need() = 0;

	// stop aquisition (has_data returns 0,
	// pending data is not affected.)
	virtual void stop() {}

	// get rid of pending data
   virtual void flush() {}

	// return 1 if device has some data waiting
	virtual int has_data() = 0;

	// return next aquired thing
	virtual void *get_data(int& out1, int& out2, 
		void*& = dummy, void*& = dummy, void*& = dummy, int& = dummy2, unsigned long& = dummy3, int no_convert = 0) = 0;

	void set_fail_reason(const char *a) {
		strncpy(fail_reason, a, sizeof(fail_reason) - 1);
		fail_reason[sizeof(fail_reason) - 1] = 0;
	}
	char *get_fail_reason() {return strdup(fail_reason);}
	virtual int generates_events() {return 0;}
	
protected:
	int inited;
	char fail_reason[255];
};

#define AQ_YUV9 1
#define AQ_RGB555 2
#define AQ_RGB24 4
#define AQ_INDEX 8
#define AQ_RED 16
#define AQ_GREEN 32
#define AQ_BLUE 64
#define AQ_ONE_PLANE 128
#define AQ_ALL_PLANES 256
#define AQ_COLOR 512
#define AQ_YUV12 1024
#define AQ_UYVY 2048
#define AQ_YUY2 4096
#define AQ_NV21 8192
#define AQ_PLANES (AQ_RED|AQ_GREEN|AQ_BLUE)
#define AQ_DEPTH (AQ_RGB555|AQ_RGB24|AQ_INDEX)
#define AQ_FORMAT (AQ_RGB555|AQ_RGB24|AQ_YUV9|AQ_INDEX|AQ_YUV12|AQ_UYVY|AQ_YUY2|AQ_NV21)
#define is_ppm(fmt) (((fmt)&AQ_COLOR) && !((fmt)&(AQ_YUV9|AQ_YUV12|AQ_YUY2|AQ_UYVY|AQ_NV21)))
#define is_color_yuv9(fmt) (((fmt)&AQ_COLOR) && ((fmt)&AQ_YUV9))
#define is_color_yuv12(fmt) (((fmt)&AQ_COLOR) && ((fmt)&AQ_YUV12|AQ_YUY2|AQ_UYVY|AQ_NV21))
#define is_color_yuv(fmt) (((fmt)&AQ_COLOR) && ((fmt)&(AQ_YUV9|AQ_YUV12|AQ_YUY2|AQ_UYVY|AQ_NV21)))
#define is_pgm(fmt) (!((fmt)&AQ_COLOR) || is_color_yuv9(fmt) || is_color_yuv12(fmt))
#define is_luma_only(fmt) (!((fmt)&AQ_COLOR))
#endif
