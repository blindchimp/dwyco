
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqvfw.cc 1.20 1999/01/10 16:09:25 dwight Checkpoint $
 */
#pragma option -w-sig
#include "vfwmgr.h"
#include "aqvfw.h"
#include "pbmcfg.h"
#include "doinit.h"
#include "vc.h"
#include "jccolor.h"
#include "sleep.h"
#include "dwrtlog.h"
#include "uicfg.h"
#include "dwtimer.h"

#define MASKFUN(x) (((((int)(x) + samp_off) ^ xor_mask) & and_mask) | or_mask)
#ifndef bcopy
#define bcopy(a,b,c) memcpy((b), (a), (c))
#endif

//
// when converting to grayscale, we assume the order is
// BGR (as microsoft crap is) for 24 bits samples 16 bit
// samples, because some boards seem to produce those
// orderings.
//


VFWAquire::VFWAquire(int groupsz) :
#if 0
    bufs(2 * groupsz, 1, 0), vhh(2 * groupsz, 1, 0), fbufs(2 * groupsz, 1, 0),
    frames(groupsz, 1, 0), fmts(2 * groupsz, 1, 0), cb_bufs(2 * groupsz, 1, 0),
    cr_bufs(2 * groupsz, 1, 0)
#else
    bufs(2, 1, 0), vhh(2, 1, 0), fbufs(2, 1, 0),
    frames(2, 1, 0), fmts(2, 1, 0), y_bufs(2, 1, 0), cb_bufs(2, 1, 0),
    cr_bufs(2, 1, 0)
#endif
{
    autoconfig = 1;
    hv = 0;
    hv_extin = 0;
    ccols = 0;
    crows = 0;
    cdepth = 0;
    for(int i = 0; i < bufs.num_elems(); ++i)
    {
        bufs[i] = 0;
        y_bufs[i] = 0;
        cb_bufs[i] = 0;
        cr_bufs[i] = 0;
        fmts[i] = 0;
    }
    next_buf = 0;
    capwnd = 0;
    vfwmgr = 0;
    memset(&capsetup, 0, sizeof(capsetup));
    memset(&caps, 0, sizeof(caps));
    next_ibuf = 0;
    streaming = 0;
    msg_box = 0;
    style = AQ_YUV9;
    upside_down = 0;
    and_mask = 0xff;
    xor_mask = 0;
    or_mask = 0;
    samp_off = 0;
    return_group = 0;
    got_frames = 0;
    nframes = groupsz;
    drool = 0;
    swap_rb = 0;
    force_gray = 0;
    swap_uv = 0;
    palinfo = 0;
}

void
VFWAquire::disconnect()
{
    stop();
}

int
VFWAquire::set_vfw_format(int cols, int rows, int depth, int palette)
{
    if(vfwmgr == 0 || capwnd == 0)
        return 0;

    BITMAPINFO bmi;
    int restart = 0;
    if(streaming)
    {
        stop();
        restart = 1;
    }
    long size = capGetVideoFormatSize(capwnd);
    BITMAPINFO *bmip = (BITMAPINFO *)malloc(size);
    if(!capGetVideoFormat(capwnd, bmip, size))
    {
        error("can't get video format");
        return 0;
    }
    // before changing anything, flush any pending frames
    // out of the buffers... note leak here, doesn't happen
    // often enough to worry about it
    pass();
#if 0
    while(has_data())
    {
        int cols, rows;
        gray **g = (gray **)get_data(cols, rows);
        pgm_freearray(g, rows);
    }
#endif

    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    if(depth == 16)
        bmi.bmiHeader.biSizeImage = cols * rows * 2;
    else if(depth == 24)
        bmi.bmiHeader.biSizeImage = cols * rows * 3;
    else if(depth == 8)
        bmi.bmiHeader.biSizeImage = cols * rows;
    else if(depth == 4)
        bmi.bmiHeader.biSizeImage = cols * rows / 2;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrImportant = 0;

    bmi.bmiHeader.biWidth = cols;
    bmi.bmiHeader.biHeight = rows;
    bmi.bmiHeader.biBitCount = depth;
    bmi.bmiHeader.biClrUsed = palette;

    int ret = capSetVideoFormat(capwnd, &bmi, sizeof(bmi));

    if(!ret)
    {
        // reset to old format
        capSetVideoFormat(capwnd, bmip, size);
    }

    if(ret)
        new_format();
    if(restart)
        need();
    return ret;
}

void
VFWAquire::new_format()
{
    if(vfwmgr == 0 || capwnd == 0)
        return;
    long size = capGetVideoFormatSize(capwnd);
    BITMAPINFO *bmip = (BITMAPINFO *)malloc(size);
    if(!capGetVideoFormat(capwnd, bmip, size))
    {
        error("can't get video format");
        return;
    }
    // before changing anything, flush any pending frames
    // out of the buffers... note, leak here if yuv or yuv12
    // but changing formats on the fly happens so infrequently
    // i'm not going to bother with it now
#if 0
    while(has_data())
    {
        int cols, rows;
        gray **g = (gray **)get_data(cols, rows);
        pgm_freearray(g, rows);
    }
#endif
    pass();

    ccols = bmip->bmiHeader.biWidth;
    crows = bmip->bmiHeader.biHeight;
    compression = bmip->bmiHeader.biCompression;
    // bogus, docs say must be 1,4,8,24 but
    // some drivers return 16 (for 16 bit depth)
    //
    cdepth = bmip->bmiHeader.biBitCount;
    is_palette = bmip->bmiHeader.biClrUsed > 0;
    if(is_palette)
    {
        if(palinfo)
            free(palinfo);
        palinfo = bmip;
        palette = (RGBQUAD *)((LPSTR)bmip + (WORD)(bmip->bmiHeader.biSize));
        set_format(AQ_INDEX);
        set_use_plane(AQ_ONE_PLANE);
    }
    else
    {
        free(bmip);
        if(palinfo)
            free(palinfo);
        palinfo = 0;
    }
}


void
VFWAquire::set_format(int s)
{
    style = (style & ~AQ_FORMAT) | (s & AQ_FORMAT);
}

void
VFWAquire::set_style(int s)
{
    style = s;
}

void
VFWAquire::add_style(int s)
{
    style |= s;
}

void
VFWAquire::remove_style(int s)
{
    style &= ~s;
}

void
VFWAquire::set_upside_down(int s)
{
    upside_down = !!s;
}

void
VFWAquire::closevid()
{
    // make sure we can't get called back with
    // bogus object pointer, whether we created the
    // window or vidinput did.
    stop();
    if(capwnd)
    {
        // not clear from docs that after an abort
        // capture may still be in progress, so
        // we do the dangerous thing and spin.
        if(!capSetUserData(capwnd, NULL))
            ;//VcError << "bad close user data\n";
        if(!capSetCallbackOnVideoStream(capwnd, NULL))
            ;//VcError << "bad close set stream\n";
    }
}

VFWAquire::~VFWAquire()
{
    stop();
    closevid();
    for(int i = 0; i < bufs.num_elems(); ++i)
    {
        delete [] bufs[i];
    }
    if(palinfo)
        free(palinfo);
}

void
VFWAquire::stop()
{
    if(streaming && capwnd)
    {
        capCaptureStop(capwnd);
        GRTLOG("stop", 0, 0);
        void pump_messages();
        DwTimer t;
        t.set_oneshot(1);
        t.load(2 * 1000L);
        t.start();

        extern int Sleeping;
        Sleeping = 1;
        while(!t.is_expired())
        {
            CAPSTATUS c;
            if(!capGetStatus(capwnd, &c, sizeof(c)))
                break;
            if(c.fCapturingNow)
            {
                GRTLOG("stop sleep", 0, 0);
                pump_messages();
            }
            else
                break;
        }
out:
        Sleeping = 0;
        GRTLOG("stop sleep = %d", t.get_time_left(), 0);
#if 0
        for(int i = 0; i < 100000; ++i)
        {
            CAPSTATUS c;
            capGetStatus(capwnd, &c, sizeof(c));
            if(c.fCapturingNow)
            {
                GRTLOG("stop sleep", 0, 0);
                ;
                dwsleep(1);

            }
            else
                break;
        }
        GRTLOG("stop sleep = %d", i, 0);
#endif
    }
    streaming = 0;
    drool = 1;
}

void
VFWAquire::error(HVIDEO h, DWORD err, char *moretxt)
{
    char errtxt[160];
    char errtxt2[80];
    strcpy(errtxt, "don't know");
    if(msg_box)
        MessageBox(parwnd, errtxt, "video for windows error", MB_OK);
    stop();
    closevid();
}

void
VFWAquire::error(const char *errtxt)
{
    if(msg_box)
        MessageBox(parwnd, errtxt, "video for windows error", MB_OK);
}

static long vlock;

// note: very primitive, you can't call spin twice
// in a row without a vunlock...
void
vspin()
{
    while(InterlockedExchange(&vlock, 1))
        Sleep(0);
}

void
vunlock()
{
    InterlockedExchange(&vlock, 0);
}

static
LRESULT CALLBACK
unload_frame(HWND capwnd, LPVIDEOHDR vh)
{
    // this pains me no end... getting the user
    // data causes a context switch to the main
    // thread, which is not what i want right here...
    // and we can't put the user data into
    // video header because we don't really have
    // guaranteed access to it. so, since i know there
    // is only ever one of these object, i just use
    // the global. multiple capture devices going at
    // the same time will require a real solution to this
    // problem.

    //VFWAquire *aq = (VFWAquire *)capGetUserData(capwnd);
    if(InterlockedExchange(&vlock, 1))
        return 1;
    VFWAquire *aq = TYPESAFE_DOWNCAST(TheAq, VFWAquire);
    if(!aq || aq->drool)
    {
        InterlockedExchange(&vlock, 0);
        return 1;
    }
    aq->unload_frame(vh);
    // note: kick the main thread to pick up the frame.
    // some drivers appear to do this for us, but some
    // don't. WM_USER+403 does nothing... just kicks it
    // out of the wait loop
    extern HWND Main_window;
    if(Main_window)
        ::PostMessage(Main_window, WM_USER+403, 0, 0);
    InterlockedExchange(&vlock, 0);
    return 1;
}

#if 0
static
LRESULT CALLBACK
yield(HWND capwnd)
{
    VFWAquire *aq = (VFWAquire *)capGetUserData(capwnd);
    if(!aq || aq->drool)
    {
        return 0;
    }
    return 1;
}
#endif

void
VFWAquire::unload_frame(LPVIDEOHDR vh)
{
    int cols, rows;
    if(next_buf == (next_ibuf + 1) % bufs.num_elems())
        return ; // drop frame
    bufs[next_ibuf] = new char[vh->dwBytesUsed];
    fmts[next_ibuf] = vh->dwBytesUsed;
    y_bufs[next_ibuf] = vh->dwTimeCaptured;
    memcpy(bufs[next_ibuf], vh->lpData, vh->dwBytesUsed);
    next_ibuf = (next_ibuf + 1) % bufs.num_elems();
}

static LRESULT CALLBACK
error(HWND hwnd, int id, LPCSTR txt)
{
    if(id == 0)
        return 0;
    VFWAquire *aq = (VFWAquire *)capGetUserData(hwnd);
    if(!aq)
        return 1;
    aq->error(txt);
    return 1;
}

int
VFWAquire::driver_connect(int devid)
{
    capwnd = capCreateCaptureWindow("Dwyco Video", WS_CHILD,
                                    0, 0, 160, 120, parwnd, 0);
    if(capwnd == NULL)
    {
        error("can't create capture");
        return 0;
    }
    if(!capDriverConnect(capwnd, devid))
    {
        error("can't connect to driver");
        capSetCallbackOnError(capwnd, NULL);
        DestroyWindow(capwnd);
        capwnd = 0;
        return 0;
    }
    capSetUserData(capwnd, this);
    capSetCallbackOnError(capwnd, ::error);
    capSetCallbackOnVideoStream(capwnd, NULL);
    if(!capDriverGetCaps(capwnd, &caps, sizeof(caps)))
    {
        error("can't get caps");
        capSetCallbackOnError(capwnd, NULL);
        capDriverDisconnect(capwnd);
        DestroyWindow(capwnd);
        capwnd = 0;
        return 0;
    }
    return 1;
}

void
VFWAquire::driver_disconnect()
{
    if(capwnd)
    {
        capSetUserData(capwnd, NULL);
        capSetCallbackOnError(capwnd, NULL);
        capSetCallbackOnVideoStream(capwnd, NULL);
        capDriverDisconnect(capwnd);
        DestroyWindow(capwnd);
        capwnd = 0;
    }
}

// this version of init assumes the
// vfwmgr stuff is responsible for
// everything except format related and
// capture related things.
int
VFWAquire::init(VFWShit *v, HWND par, int ms_per_frame, int autoconfig)
{
    DWORD err;
    parwnd = par;
    if(v == 0 || v->capwnd == 0)
    {
        set_fail_reason("video is turned off");
        return 0;
    }
    vfwmgr = v;
    capwnd = v->capwnd;

    if(!capCaptureGetSetup(capwnd, &capsetup, sizeof(capsetup)))
    {
        error("can't get setup");
        return 0;
    }
    capsetup.dwRequestMicroSecPerFrame = ms_per_frame * 1000L;
    capsetup.fMakeUserHitOKToCapture = 0;
    capsetup.wPercentDropForError = 200;
    capsetup.fLimitEnabled = 0;
    capsetup.fYield = 1;
    capsetup.wNumVideoRequested = 2;
    capsetup.fAbortLeftMouse = 0;
    capsetup.fAbortRightMouse = 0;
    capsetup.fCaptureAudio = 0;
    capsetup.fMCIControl = 0;
    capsetup.vKeyAbort = 0;
#if 0
    capsetup.AVStreamMaster = AVSTREAMMASTER_NONE;
#endif
    if(!capCaptureSetSetup(capwnd, &capsetup, sizeof(capsetup)))
    {
        error("can't set setup");
        return 0;
    }
    capSetUserData(capwnd, this);
    capSetCallbackOnVideoStream(capwnd, ::unload_frame);
    //capSetCallbackOnYield(capwnd, ::yield);
    new_format();

#if 0
    if(!(ccols % 4 == 0 && ccols <= 160 && crows <= 120))
    {
#define FORMAT_ERR "shareware 16-bit version only codes <= 160x120, width (mod 4) == 0\n" \
	"Use the \"Video Input\" configuration to set the video format to the proper size."
        if(msg_box)
            MessageBox(parwnd, FORMAT_ERR, "codec error", MB_OK);
        strcpy(fail_reason, FORMAT_ERR);
        closevid();
        return 0;
    }
#endif
    inited = 1;
    return 1;
}
int
VFWAquire::init(int devid, HWND par, int ms_per_frame, int autoconfig)
{
    DWORD err;
    parwnd = par;
    if(!driver_connect(devid))
        return 0;

    if(!capPreview(capwnd, 0))
    {
        error("can't disable preview");
        driver_disconnect();
        return 0;
    }
    if(!capCaptureGetSetup(capwnd, &capsetup, sizeof(capsetup)))
    {
        error("can't get setup");
        driver_disconnect();
        return 0;
    }
    capsetup.dwRequestMicroSecPerFrame = ms_per_frame * 1000L;
    capsetup.fMakeUserHitOKToCapture = 0;
    capsetup.wPercentDropForError = 99;
    capsetup.fLimitEnabled = 0;
    capsetup.fYield = 1;
    capsetup.wNumVideoRequested = 2;
    capsetup.fAbortLeftMouse = 0;
    capsetup.fAbortRightMouse = 0;
    capsetup.fCaptureAudio = 0;
#if 0
    capsetup.AVStreamMaster = AVSTREAMMASTER_NONE;
#endif
    if(!capCaptureSetSetup(capwnd, &capsetup, sizeof(capsetup)))
    {
        error("can't set setup");
        driver_disconnect();
        return 0;
    }
    if(!capCaptureGetSetup(capwnd, &capsetup, sizeof(capsetup)))
    {
        error("can't get setup");
        driver_disconnect();
        return 0;
    }
    capSetCallbackOnVideoStream(capwnd, ::unload_frame);
    long size = capGetVideoFormatSize(capwnd);
    BITMAPINFO *bmip = (BITMAPINFO *)malloc(size);
    if(!capGetVideoFormat(capwnd, bmip, size))
    {
        error("can't get video format");
        driver_disconnect();
        return 0;
    }
    ccols = bmip->bmiHeader.biWidth;
    crows = bmip->bmiHeader.biHeight;
    // bogus, docs say must be 1,4,8,24 but
    // some drivers return 16 (for 16 bit depth)
    //
    cdepth = bmip->bmiHeader.biBitCount;
    is_palette = bmip->bmiHeader.biClrUsed > 0;
    if(is_palette)
    {
        palette = (RGBQUAD *)((LPSTR)bmip + (WORD)(bmip->bmiHeader.biSize));
        set_format(AQ_INDEX);
        set_use_plane(AQ_ONE_PLANE);
    }

#if 0
    if(!(ccols % 4 == 0 && ccols <= 160 && crows <= 120))
    {
#define FORMAT_ERR "shareware 16-bit version only codes <= 160x120, width (mod 4) == 0\n" \
	"Use the \"Video Input\" configuration to set the video format to the proper size."
        if(msg_box)
            MessageBox(parwnd, FORMAT_ERR, "codec error", MB_OK);
        strcpy(fail_reason, FORMAT_ERR);
        closevid();
        return 0;
    }
#endif
    inited = 1;
    return 1;
}

void
VFWAquire::need()
{
    if(!streaming)
    {
        if(vfwmgr)
        {
            vfwmgr->preview_off();
        }
        streaming = 1;
        drool = 0;
        capCaptureSequenceNoFile(capwnd);
    }
    else
    {
        CAPSTATUS c;
        if(!capGetStatus(capwnd, &c, sizeof(c)))
        {
            GRTLOG("need: can't get status", 0, 0);
            return;
        }
        if(!c.fCapturingNow)
        {
            GRTLOG("capture stopped externally", 0, 0);
            //capCaptureSequenceNoFile(capwnd);
        }
    }
}

void
VFWAquire::pass()
{
    int cols, rows;
    void *g;
    void *y, *cb, *cr;
    int fmt;
    vspin();
    while(next_buf != next_ibuf)
    {
        delete [] bufs[next_buf];
        bufs[next_buf] = 0;
        GRTLOG("chuck %d", (int)y_bufs[next_buf], 0);
        next_buf = (next_buf + 1) % bufs.num_elems();
    }
    vunlock();
}

// note: need to fix this: if you want groups
// of frames and you call has_data has_data
// instead of has_data get_data has_data you
// lose intervening frames.
//
int
VFWAquire::has_data()
{
    vspin();
    if(next_buf == next_ibuf)
    {
        vunlock();
        return 0;
    }
    vunlock();
    return 1;
}

static gray far Colgray15[32768U];
static gray far Colgray15_s[32768U];
static int far m30[256];
static int far m59[256];
static int far m11[256];


static void
gen_table()
{
    int i;
    for(i = 0; i < 32; ++i)
        for(int j = 0; j < 32; ++j)
            for(int k = 0; k < 32; ++k)
            {
                int idx = k | (j << 5) | (i << 10);
                int idx2 = (k << 10) | (j << 5) | i;
                Colgray15[idx] = ((30 * i + 59 * j + 11 * k) * 8) / 100;
                Colgray15_s[idx2] = ((30 * k + 59 * j + 11 * i) * 8) / 100;
            }
    // note: multiplied by 1.28 so that >>7 works to divide
    // by 128...
    for(i = 0; i < 256; ++i)
    {
        m30[i] = i * 38;
        m59[i] = i * 76;
        m11[i] = i * 14;
    }
}

void *
VFWAquire::get_data(int& cols, int& rows, void*& y, void*& cb, void*& cr, int& fmt,
                    unsigned long& captime, int no_convert)
{
    vspin();
    if(next_buf != next_ibuf)
    {
        cols = ccols;
        rows = crows;
        int nb = next_buf;
        VIDEOHDR vh;
        vh.lpData = (unsigned char *)bufs[nb];
        vh.dwBytesUsed = fmts[nb];
        captime = y_bufs[nb];
        bufs[nb] = 0;
        next_buf = (next_buf + 1) % bufs.num_elems();
        vunlock();
        GRTLOG("conv %d", (int)y_bufs[nb], 0);
        void *ret = convert_data(&vh, cols, rows, y, cb, cr, fmt, no_convert);
        GRTLOG("conv done", 0, 0);
        delete [] vh.lpData;
        return ret;
    }
    vunlock();
    oopanic("aqvfw get no data");
    // not reached
    return 0;
}

void *
VFWAquire::convert_data(LPVIDEOHDR vhp, int& cols, int& rows,
                        void*& y, void*& cb, void*& cr, int& fmt, int no_ycbcr)
{
    cr = 0;
    cb = 0;
    y = 0;
    fmt = 0;
    cols = 0;
    rows = 0;
    if(return_group)
    {
        PGMIMG *a = new PGMIMG[nframes];
        for(int i = 0; i < nframes; ++i)
            a[i] = (PGMIMG)frames[i];
        cols = ccols;
        rows = crows;
        return_group = 0;
        got_frames = 0;
        return (void *)a;
    }

    static int been_here;
    if(!been_here)
    {
        gen_table();
        been_here = 1;
    }
    if(force_gray)
        remove_style(AQ_COLOR);
    // convert buffer from whatever it is (seems to
    // be pretty random what various vendors provide)
    // to 8 bit pgm for now...
    int bytes_got = vhp->dwBytesUsed;
    if(autoconfig)
    {
        // attempt to figure it out based on size first

        if(bytes_got == ccols * crows * 3)
            set_format(AQ_RGB24);
        // voodoo numbers: mmiofourcc compression numbers for UYVY and YUY2
        else if(bytes_got == ccols * crows * 2 &&
                (compression != 0x59565955 && compression != 0x32595559))
            set_format(AQ_RGB555);
        else if(bytes_got == ccols * crows * 2 && compression == 0x59565955)
        {
            set_format(AQ_UYVY);
            upside_down = 1;
            swap_uv = 0;
        }
        else if(bytes_got == ccols * crows * 2 && compression == 0x32595559)
        {
            set_format(AQ_YUY2);
            upside_down = 1;
            swap_uv = 0;
        }
        else if(bytes_got == ccols * crows || bytes_got == ccols * crows / 2)
            set_format(AQ_INDEX);
        else if(bytes_got == ccols * crows + 2 * ccols * crows / 4)
        {
            set_format(AQ_YUV12);
            upside_down = 1;
            swap_uv = 1;
        }
        else if(bytes_got == ccols * crows + 2 * ccols * crows / 16)
        {
            set_format(AQ_YUV9);
            upside_down = 1;
            swap_uv = 0;
        }
        else if(cdepth == 16)
        {
            set_format(AQ_RGB555);
        }
        else if(cdepth == 24)
        {
            set_format(AQ_RGB24);
        }
        else if(cdepth == 8 || cdepth == 4)
        {
            // palette member already setup at init time
            set_format(AQ_INDEX);
        }
        else
        {
            // some oddball format
            // pick yuv9 just in case
            set_format(AQ_YUV9);
        }
    }

    gray **g = 0;
    pixel **p = 0;

    // yuv9 returns 3 gray planes instead of a PPM.
    // this gives the app the opportunity to avoid
    // a color conversion.
    if((style & AQ_COLOR) && !(style & (AQ_YUV9|AQ_YUV12|AQ_UYVY|AQ_YUY2)))
        p = ppm_allocarray(ccols, crows);
    else
        g = pgm_allocarray(ccols, crows);

    unsigned short *b = (unsigned short *)vhp->lpData;

    unsigned char *c = (unsigned char *)b;
    long bytes_needed = (long)ccols * (long)crows * 3L;
    if(style & AQ_YUV9)
    {
        bytes_needed = (long)ccols * (long)crows;
        if(style & AQ_COLOR)
            bytes_needed += 2 * ((ccols * crows) / 16);
    }
    else if(style & AQ_YUV12)
    {
        bytes_needed = (long)ccols * (long)crows;
        if(style & AQ_COLOR)
            bytes_needed += 2 * ((ccols * crows) / 4);
    }
    else if(style & AQ_RGB555)
        bytes_needed = (long)ccols * (long)crows * 2L;
    else if(style & AQ_RGB24)
        bytes_needed = (long)ccols * (long)crows * 3L;
    else if(style & AQ_INDEX)
    {
        if(cdepth == 8)
            bytes_needed = (long)ccols * (long)crows;
        else if(cdepth == 4)
            bytes_needed = ((long)ccols * (long)crows) / 2L; // assumed packed

    }
    else if(style & (AQ_UYVY|AQ_YUY2))
    {
        bytes_needed = ccols * crows + 2 * (ccols / 2) * crows;
    }

    if(vhp->dwBytesUsed < bytes_needed)
    {
	// not enough data available to do the requested conversion
	if(p) memset(&p[0][0], 0, ccols * crows * 3);
	if(g) memset(&g[0][0], 0, ccols * crows);
    }
    else if(style & (AQ_YUV9|AQ_YUV12))
    {
        // yuv9, not upside down, all luma first
        // usually this will be upside down because
        // even tho the YUV formats come in rightside-up
        // if we don't flip it here, we end up doing it
        // before display, since all the fucker-microsoft
        // RGB bitmap stuff is upside down.
        if((autoconfig && !upside_down) || (!autoconfig && upside_down))
        {
            if(and_mask == 0xff && or_mask == 0 && xor_mask == 0 && samp_off == 0)
            {
                bcopy(c, &g[0][0], ccols * crows);
                c += ccols * crows;
            }
            else
            {
                for(int i = 0; i < crows; ++i)
                {
                    gray *gr = &g[i][0];
                    for(int j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                    }
                }
            }
        }
        else
        {
            if(and_mask == 0xff && or_mask == 0 && xor_mask == 0 && samp_off == 0)
            {
                for(int i = crows - 1; i >= 0; --i)
                {
                    gray *gr;
                    gr = &g[i][0];
                    bcopy(c, gr, ccols);
                    c += ccols;
                }
            }
            else
            {
                for(int i = crows - 1; i >= 0; --i)
                {
                    gray *gr = &g[i][0];
                    for(int j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                        *gr++ = MASKFUN(*c++);
                    }
                }
            }
        }
        // c points to first chroma plane, which is subsampled
        // by 4 in both directions from the luma plane.
        // it's the app's job to know the format and what
        // to do about it.
        if(style & AQ_COLOR)
        {
            cr = get_chroma_plane(c, (style & AQ_YUV12) ? 2 : 4);
            cb = get_chroma_plane(c, (style & AQ_YUV12) ? 2 : 4);
            if((autoconfig && swap_uv) ||
                    (!autoconfig && (style & AQ_YUV12) && !swap_uv) ||
                    (!autoconfig && (style & AQ_YUV9) && swap_uv))
            {
                void *tmp = cr;
                cr = cb;
                cb = tmp;
            }
        }
    }
    else if((style & AQ_UYVY) || (style & AQ_YUY2))
    {
        gray **cbo = 0;
        gray **cro = 0;
        if(style & AQ_COLOR)
        {
            cbo = pgm_allocarray(ccols / 2, crows / 2);
            cro = pgm_allocarray(ccols / 2, crows / 2);
        }
        unpack_to_planar(c, g, cbo, cro);
        if(!autoconfig && swap_uv)
        {
            cr = cbo;
            cb = cro;
        }
        else
        {
            cb = cbo;
            cr = cro;
        }
    }
    else if((style & AQ_ONE_PLANE) && !(style & AQ_COLOR))
        grab_one_plane(c, g);
    else if((style & AQ_ALL_PLANES) && !(style & AQ_COLOR))
        rgb_to_gray(c, g);
    else if(style & AQ_COLOR)
    {
        if(no_ycbcr)
        {
            unpack_to_ppm(c, p, 0, 0, 0, 1);
            y = 0;
            cb = 0;
            cr = 0;
        }
        else
        {
            gray **yo = pgm_allocarray(ccols, crows);
            gray **cbo = pgm_allocarray(ccols, crows);
            gray **cro = pgm_allocarray(ccols, crows);
            unpack_to_ppm(c, p, yo, cbo, cro, 0);
            // note: no swap uv here since the input was rgb
            y = yo;
            cb = cbo;
            cr = cro;
        }
    }

    cols = ccols;
    rows = crows;
    fmt = style;
    if(style & (AQ_YUV9|AQ_YUV12|AQ_UYVY|AQ_YUY2))
        return g;
    if(style & AQ_COLOR)
        return p;
    return g;
}

void
VFWAquire::grab_one_plane(unsigned char *buf, gray **g)
{
    // grab one plane (set saturation to 0)
    unsigned short *b = (unsigned short *)buf;
    unsigned char *c = (unsigned char *)buf;
    int clroff = 0;
    // assume c is pointing to a blue sample, unless
    // swapping is spec'ed for red&blue
    switch(style & (AQ_PLANES | AQ_DEPTH))
    {
    case AQ_BLUE|AQ_RGB24:
        if(swap_rb)
            c += 2;
        break;
    case AQ_GREEN|AQ_RGB24:
        ++c;
        break;
    case AQ_RED|AQ_RGB24:
        if(!swap_rb)
            c += 2;
        break;
    // note: this is bad, assumes layout of RGBQUAD struct
    // is packed bytes in bgr order. note: technically, the
    // swap item shouldn't be used here since the
    // order of the rgbquad struct is spec'ed, but for
    // flexibility, we do it anyway.
    case AQ_RED|AQ_INDEX:
        if(!swap_rb)
            clroff = 2;
        break;
    case AQ_GREEN|AQ_INDEX:
        clroff = 1;
        break;
    case AQ_BLUE|AQ_INDEX:
        if(swap_rb)
            clroff = 2;
        break;
    }
    if((style & AQ_INDEX) && !palette)
        return;

    int do_mask = (and_mask != 0xff || or_mask != 0 || xor_mask != 0 || samp_off != 0);

    int new_style = style;
    if(swap_rb)
    {
        switch(style & (AQ_PLANES | AQ_DEPTH))
        {
        case AQ_RED|AQ_RGB555:
            new_style = (style & ~(AQ_PLANES|AQ_DEPTH)) | (AQ_BLUE|AQ_RGB555);
            break;
        case AQ_BLUE|AQ_RGB555:
            new_style = (style & ~(AQ_PLANES|AQ_DEPTH)) | (AQ_RED|AQ_RGB555);
            break;
        }
    }

    for(int i = 0; i < crows; ++i)
    {
        gray *gr;
        int j;
        if(upside_down)
            gr = &g[crows - i - 1][0];
        else
            gr = &g[i][0];
        switch(new_style & (AQ_PLANES | AQ_DEPTH))
        {
        default:
            // hmm, masks aren't set right, just don't do
            // anything in order to avoid a possible crash.
            return;
        case AQ_BLUE|AQ_RGB555:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                    *gr++ = MASKFUN((*b++ & 0x1f) << 3);
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = (*b++ & 0x1f) << 3;
                    *gr++ = (*b++ & 0x1f) << 3;
                    *gr++ = (*b++ & 0x1f) << 3;
                    *gr++ = (*b++ & 0x1f) << 3;
                }
            }
            break;

        case AQ_GREEN|AQ_RGB555:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                    *gr++ = MASKFUN((*b++ & (0x1f << 5)) >> 2);
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                    *gr++ = (*b++ & (0x1f << 5)) >> 2;
                }
            }
            break;

        case AQ_RED|AQ_RGB555:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                    *gr++ = MASKFUN((*b++ >> 7) & 0xf8);
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = (*b++ >> 7) & 0xf8;
                    *gr++ = (*b++ >> 7) & 0xf8;
                    *gr++ = (*b++ >> 7) & 0xf8;
                    *gr++ = (*b++ >> 7) & 0xf8;
                }
            }
            break;
        case AQ_BLUE|AQ_RGB24:
        // FALL THROUGH
        case AQ_GREEN|AQ_RGB24:
        // FALL THROUGH
        case AQ_RED|AQ_RGB24:
            if(do_mask)
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = MASKFUN(c[0]);
                    *gr++ = MASKFUN(c[3]);
                    *gr++ = MASKFUN(c[6]);
                    *gr++ = MASKFUN(c[9]);
                    c += 12;
                }
            }
            else
            {
                for(j = 0; j < ccols; j += 4)
                {
                    *gr++ = c[0];
                    *gr++ = c[3];
                    *gr++ = c[6];
                    *gr++ = c[9];
                    c += 12;
                }
            }
            break;

        case AQ_BLUE|AQ_INDEX:
        case AQ_GREEN|AQ_INDEX:
        case AQ_RED|AQ_INDEX:
            if(do_mask)
            {
                if(cdepth == 8)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[0]] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[1]] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[2]] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[3]] + clroff));
                        c += 4;
                    }
                }
                else if(cdepth == 4)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[0] >> 4] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[0] & 0xf] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[1] >> 4] + clroff));
                        *gr++ = MASKFUN(*((unsigned char *)&palette[c[1] & 0xf] + clroff));
                        c += 2;
                    }
                }
            }
            else
            {
                if(cdepth == 8)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = *((unsigned char *)&palette[c[0]] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[1]] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[2]] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[3]] + clroff);
                        c += 4;
                    }
                }
                else if(cdepth == 4)
                {
                    for(j = 0; j < ccols; j += 4)
                    {
                        *gr++ = *((unsigned char *)&palette[c[0] >> 4] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[0] & 0xf] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[1] >> 4] + clroff);
                        *gr++ = *((unsigned char *)&palette[c[1] & 0xf] + clroff);
                        c += 2;
                    }
                }
            }
            break;

        }
    }
}

void
VFWAquire::rgb_to_gray(unsigned char *buf, gray **g)
{
    unsigned short *b = (unsigned short *)buf;
    unsigned char *c = (unsigned char *)buf;
    int do_mask = (and_mask != 0xff || or_mask != 0 || xor_mask != 0 || samp_off != 0);
    if((style & AQ_INDEX) && !palette)
        return;
    // note: this looks backwards because it is. the stuff down
    // below was mistakenly swapped, and this is "unswapping" it.
    //
    gray *conv555 = !swap_rb ? Colgray15_s : Colgray15;
    int *bluemul = !swap_rb ? m30 : m11;
    int *redmul = !swap_rb ? m11 : m30;
    for(int i = 0; i < crows; ++i)
    {
        gray *gr;
        if(upside_down)
            gr = &g[crows - i - 1][0];
        else
            gr = &g[i][0];
        if(style & AQ_RGB555)
        {
            for(int j = 0; j < ccols; j += 4)
            {
                gray g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
                g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
                g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
                g2 = conv555[*b++ & (unsigned short)0x7fff];
                *gr++ = g2;
            }
        }
        else if(style & AQ_RGB24)
        {
            if(do_mask)
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    *gr++ = (redmul[MASKFUN(c[0])] + m59[MASKFUN(c[1])] + bluemul[MASKFUN(c[2])]) >> 7;
                    *gr++ = (redmul[MASKFUN(c[3])] + m59[MASKFUN(c[4])] + bluemul[MASKFUN(c[5])]) >> 7;
                    *gr++ = (redmul[MASKFUN(c[6])] + m59[MASKFUN(c[7])] + bluemul[MASKFUN(c[8])]) >> 7;
                    *gr++ = (redmul[MASKFUN(c[9])] + m59[MASKFUN(c[10])] + bluemul[MASKFUN(c[11])]) >> 7;
                    c += 12;
                }
            }
            else
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    *gr++ = (redmul[c[0]] + m59[c[1]] + bluemul[c[2]]) >> 7;
                    *gr++ = (redmul[c[3]] + m59[c[4]] + bluemul[c[5]]) >> 7;
                    *gr++ = (redmul[c[6]] + m59[c[7]] + bluemul[c[8]]) >> 7;
                    *gr++ = (redmul[c[9]] + m59[c[10]] + bluemul[c[11]]) >> 7;
                    c += 12;
                }
            }
        }
        else if(style & AQ_INDEX)
        {
            if(do_mask)
            {
                if(cdepth == 8)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[c[0]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[1]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[2]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[3]];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        c += 4;

                    }
                }
                else if(cdepth == 4)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[(c[0] >> 4) & 0xf];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[(c[0] & 0xf)];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[(c[1] >> 4) & 0xf];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        map = &palette[c[1] & 0xf];
                        *gr++ = MASKFUN((redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7);
                        c += 2;
                    }
                }
            }
            else
            {
                if(cdepth == 8)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[c[0]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[1]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[2]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[3]];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        c += 4;
                    }
                }
                else if(cdepth == 4)
                {
                    for(int j = 0; j < ccols; j += 4)
                    {
                        RGBQUAD *map;
                        map = &palette[(c[0] >> 4) & 0xf];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[(c[0] & 0xf)];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[(c[1] >> 4) & 0xf];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        map = &palette[c[1] & 0xf];
                        *gr++ = (redmul[map->rgbRed] + m59[map->rgbGreen] + bluemul[map->rgbBlue]) >> 7;
                        c += 2;
                    }
                }
            }
        }
    }
}

#define PPM_ASSIGN_SWAP(a, r, g, b) PPM_ASSIGN((a), (b), (g), (r))
void
VFWAquire::unpack_to_ppm(unsigned char *buf, pixel **g,
                         gray **yo, gray **cbo, gray **cro, int no_ycbcr)
{
    unsigned short *b = (unsigned short *)buf;
    unsigned char *c = (unsigned char *)buf;
    int do_mask = (and_mask != 0xff || or_mask != 0 || xor_mask != 0 || samp_off != 0);
    if((style & AQ_INDEX) && !palette)
        return;
    JSAMPIMAGE a = new JSAMPARRAY[3];
    a[0] = yo;
    a[1] = cbo;
    a[2] = cro;
    for(int i = 0; i < crows; ++i)
    {
        pixel *gr;
        pixel **grs;
        if(upside_down)
        {
            gr = &g[crows - i - 1][0];
            grs = &g[crows - i - 1];
        }
        else
        {
            gr = &g[i][0];
            grs = &g[i];
        }
        if(style & AQ_RGB555)
        {
            if(swap_rb)
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN_SWAP(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                                    (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                }
            }
            else
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                    PPM_ASSIGN(*gr, (*b & 0x1f) << 3, (*b & (0x1f << 5)) >> 2,
                               (*b >> 7) & 0xf8);
                    gr++;
                    b++;
                }
            }
        }
        else if(style & AQ_RGB24)
        {
            if(swap_rb)
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN_SWAP(*gr, c[0], c[1], c[2]);
                    gr++;
                    PPM_ASSIGN_SWAP(*gr, c[3], c[4], c[5]);
                    gr++;
                    PPM_ASSIGN_SWAP(*gr, c[6], c[7], c[8]);
                    gr++;
                    PPM_ASSIGN_SWAP(*gr, c[9], c[10], c[11]);
                    gr++;
                    c += 12;
                }
            }
            else
            {
                for(int j = 0; j < ccols; j += 4)
                {
                    PPM_ASSIGN(*gr, c[0], c[1], c[2]);
                    gr++;
                    PPM_ASSIGN(*gr, c[3], c[4], c[5]);
                    gr++;
                    PPM_ASSIGN(*gr, c[6], c[7], c[8]);
                    gr++;
                    PPM_ASSIGN(*gr, c[9], c[10], c[11]);
                    gr++;
                    c += 12;
                }
            }
        }
        else if(style & AQ_INDEX)
        {
            {
                if(cdepth == 8)
                {
                    if(swap_rb)
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[c[0]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[2]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[3]];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 4;
                        }
                    }
                    else
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[c[0]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[2]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[3]];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 4;
                        }
                    }
                }
                else if(cdepth == 4)
                {
                    if(swap_rb)
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[(c[0] >> 4) & 0xf];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[0] & 0xf)];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[1] >> 4) & 0xf];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1] & 0xf];
                            PPM_ASSIGN_SWAP(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 2;
                        }
                    }
                    else
                    {
                        for(int j = 0; j < ccols; j += 4)
                        {
                            RGBQUAD *map;
                            map = &palette[(c[0] >> 4) & 0xf];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[0] & 0xf)];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[(c[1] >> 4) & 0xf];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            map = &palette[c[1] & 0xf];
                            PPM_ASSIGN(*gr, map->rgbRed, map->rgbGreen, map->rgbBlue);
                            gr++;
                            c += 2;
                        }
                    }
                }
            }
        }
        if(!no_ycbcr)
        {
            // perform YCrCb conversion right now
            // to take advantage of cache
            //
            rgb_ycc_convert((unsigned char **)grs, a,
                            upside_down ? (crows - i - 1) : i , ccols, 1);
        }
    }
    delete [] a;
}

gray **
VFWAquire::get_chroma_plane(unsigned char *&c, int subsample)
{
    gray **p;
    int ch_cols = ccols / subsample;
    int ch_rows = crows / subsample;

    p = pgm_allocarray(ch_cols, ch_rows);

    if((autoconfig && !upside_down) || (!autoconfig && upside_down))
    {
        bcopy(c, &p[0][0], ch_cols * ch_rows);
        c += ch_cols * ch_rows;
    }
    else
    {
        for(int i = ch_rows - 1; i >= 0; --i)
        {
            gray *gr;
            gr = &p[i][0];
            bcopy(c, gr, ch_cols);
            c += ch_cols;
        }
    }
    return p;
}

// pass cbo = 0 if you just want the luma samples
void
VFWAquire::unpack_to_planar(unsigned char *buf,
                            gray **yo, gray **cbo, gray **cro)
{
    // uyvy, subsampled chroma horizontally , but not vertically
    int flip = 0;
    if(upside_down)
    {
        flip = 1;
    }
    int yoff = 1;
    int cboff = 0;
    int croff = 2;
    if(style & AQ_YUY2)
    {
        yoff = 0;
        cboff = 1;
        croff = 3;
    }
    for(int i = 0; i < crows; ++i)
    {
        gray *yline;
        gray *cbline;
        gray *crline;
        if(flip)
        {
            yline = &yo[crows - i - 1][0];
            if(cbo)
            {
                cbline = &cbo[crows / 2 - i / 2 - 1][0];
                crline = &cro[crows / 2 - i / 2 - 1][0];
            }
        }
        else
        {
            yline = &yo[i][0];
            if(cbo)
            {
                cbline = &cbo[i / 2][0];
                crline = &cro[i / 2][0];
            }
        }
        int j;
        for(j = 0; j < ccols; ++j)
        {
            *yline++ = buf[j * 2 + yoff];
        }
        if(cbo)
        {
            for(j = 0; j < ccols / 2; ++j)
            {
                if(i & 1)
                {
                    *cbline = (*cbline + buf[j * 4 + cboff]) / 2;
                    *crline = (*crline + buf[j * 4 + croff]) / 2;
                    ++cbline;
                    ++crline;
                }
                else
                {
                    *cbline++ = buf[j * 4 + cboff];
                    *crline++ = buf[j * 4 + croff];
                }
            }
        }
        buf += 2 * ccols;

    }
}
