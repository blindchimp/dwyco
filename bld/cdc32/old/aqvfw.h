
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#if defined(_Windows) && defined(USE_VFW)
/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqvfw.h 1.25 1999/01/10 16:10:40 dwight Checkpoint $
 */
#ifndef AQVFW_H
#define AQVFW_H
#ifndef _MSC_VER
#include <mem.h>
#endif
#include <stddef.h>
#include <windows.h>
#include <mmsystem.h>
#include <vfw.h>
#include "vidaq.h"
#include "dwvec.h"
#include "dwvecp.h"
#include "pbmcfg.h"
#include "aq.h"
class VFWShit;

class VFWAquire : public VidAcquire
{
    friend void *
    __stdcall
    vgget_data(
        void *aqext,
        int *r_out, int *c_out,
        void **y_out, void **cr_out, void **cb_out, int *fmt_out, unsigned long *captime_out);
public:
    VFWAquire(int groupsz = 2);
    ~VFWAquire();

    int generates_events() {
        return 1;
    }
    int init(int devid, HWND par, int ms_per_frame, int autoconfig = 1);
    int init(VFWShit *v, HWND par, int ms_per_frame, int autoconfig = 1);
    void need();
    void pass();
    void stop();
    int has_data();
    void *get_data(int& cols, int& rows,
                   void*& = VidAcquire::dummy, void*& = VidAcquire::dummy, void*&  = VidAcquire::dummy,
                   int& = VidAcquire::dummy2,
                   unsigned long& = VidAcquire::dummy3,
                   int no_convert = 0);
    void disconnect();
    void new_format();
    int set_vfw_format(int cols, int rows, int depth, int palette);

    HVIDEO hv;
    HVIDEO hv_extin;
    int ccols;
    int crows;
    int cdepth;
    int autoconfig;
    int swap_rb;
    int swap_uv;
    VFWShit *vfwmgr;
    DWORD compression; // really the format

    DwVec<HGLOBAL> vhh;
    DwVec<HGLOBAL> fbufs;
    DwVec<void *> bufs;
    DwVec<int> fmts;
    DwVec<unsigned long> y_bufs; // time in milliseconds each frame was captured (from beginning of stream).
    DwVec<void *> cr_bufs;
    DwVec<void *> cb_bufs;
    int next_buf;
    int streaming;
    HWND parwnd;
    int msg_box;

    void set_format(int);
    void set_use_plane(int i) {
        style &= ~(AQ_ALL_PLANES|AQ_ONE_PLANE);
        style |= i;
    }
    void set_which_plane(int i) {
        style &= ~AQ_PLANES;
        style |= i;
    }
    void set_style(int);
    void add_style(int);
    void remove_style(int);
    void set_upside_down(int);
    unsigned int xor_mask;
    unsigned int and_mask;
    unsigned int or_mask;
    int samp_off;
    int force_gray;

private:
    int style;
    int upside_down;
    int return_group;
    DwVec<void *> frames;
    int nframes;
    int got_frames;
    RGBQUAD *palette;
    int is_palette;
    int drool;

    friend LRESULT CALLBACK unload_frame(HWND capwnd, LPVIDEOHDR vh);
    friend LRESULT CALLBACK error(HWND capwnd, int id, LPCSTR txt);
    friend LRESULT CALLBACK yield(HWND);

    HWND capwnd;
    CAPDRIVERCAPS caps;
    CAPTUREPARMS capsetup;
    BITMAPINFO *palinfo;
    int next_ibuf;
    void *convert_data(LPVIDEOHDR vhp, int& cols, int& rows, void*& y, void*& cb, void*& cr, int& fmt, int no_ycbcr);
    void unload_frame(LPVIDEOHDR vh);
    int driver_connect(int devid);
    void driver_disconnect();

    void error(const char *msg);
    void error(DWORD err, char *msg);
    void error(HVIDEO hv, DWORD err, char *msg);
    void rgb_to_gray(unsigned char *buf, gray **g);
    void grab_one_plane(unsigned char *buf, gray **g);
    void closevid();
    gray **get_chroma_plane(unsigned char *&, int subsamp);
    void unpack_to_ppm(unsigned char *in, pixel **out, gray **y, gray **cb, gray **cr, int no_ycbcr);
    void unpack_to_planar(unsigned char *in, gray **y, gray **cb, gray **cr);


};

void vspin();
void vunlock();

#endif
#endif
