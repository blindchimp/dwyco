
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "v4lcapexp.h"
#include "dwvecp.h"
#include "vidaq.h"
#include <pthread.h>
#include "dwstr.h"
#include <stdio.h>
#include <linux/videodev2.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <libv4l2.h>

//---------------------------------------------------------------------------
/*
typedef void DWYCOCALLCONV (*DwycoVVCallback)(void *);
typedef int DWYCOCALLCONV (*DwycoIVCallback)(void *);
typedef void * DWYCOCALLCONV (*DwycoVidGetDataCallback)(void *,
	int *r, int *c,
	void **y, void **cr, void **cb, int *fmt, unsigned long *captime);
DWYCOEXPORT void dwyco_set_external_video_capture_callbacks(
	DwycoVVCallback nw,
	DwycoVVCallback del,
	DwycoIVCallback init,
	DwycoIVCallback has_data,
	DwycoVVCallback need,
	DwycoVVCallback pass,
	DwycoVVCallback stop,
	DwycoVidGetDataCallback get_data
);
*/

#define NB_BUFFER 4
static DwVecP<unsigned char> mem(NB_BUFFER, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);

static DwVecP<unsigned char> bufs(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
static DwVec<unsigned long> y_bufs(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
static DwVec<unsigned int> lens(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
static int next_ibuf;
static int next_buf;
static int Selected_format;
static int V4L_grabber = -1;
static int capexit;
static pthread_t v4l_thread;
static pthread_mutex_t buf_mutex;
static void *grab_frames(void *);
static int debug = 1;

#define ioctl v4l2_ioctl
#define open v4l2_open
#define mmap v4l2_mmap
#define close v4l2_close

struct v4l_setup
{
    DwString name;
    DwString dev;
    int cols;
    int rows;
    DwString vidstd;
    int cap_format;
    int v4l_format;
};

DwVec<v4l_setup> Devs;
static int Cur_dev;


static int
probe_v4l(const char *dev, int cols, int rows, DwString& name_out, int& cap_format_out, int& v4l_format )
{

    int ret = 0;
    int fd;

    if ((fd = open (dev, O_RDWR)) == -1) {
        perror ("ERROR opening V4L interface \n");
        return 0;
    }
    struct v4l2_capability cap;
    memset (&cap, 0, sizeof (cap));
    ret = ioctl (fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {

        goto fatal;
    }

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
        goto fatal;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {

        goto fatal;
    }


    static int formats[] = {
        //V4L2_PIX_FMT_YUV410,
        V4L2_PIX_FMT_YUV420,
        V4L2_PIX_FMT_RGB24,
        V4L2_PIX_FMT_UYVY,
        V4L2_PIX_FMT_YUYV
    };
    static const char *nice_names[] = {
        "YUV420",
        "RGB24",
        "UYVY",
        "YUYV"
    };
#define FMTS (sizeof(formats)/sizeof(formats[0]))
    int i;
    struct v4l2_format fmt;
    for(i = 0; i < FMTS; ++i)
    {
        memset (&fmt, 0, sizeof (fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = cols;
        fmt.fmt.pix.height = rows;
        fmt.fmt.pix.pixelformat = formats[i];
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        ret = ioctl (fd, VIDIOC_S_FMT, &fmt);
        if (ret < 0) {
            fprintf (stderr, " format %s (%d, %d) unavailable\n", nice_names[i], cols, rows);
            continue;
        }
        if ((fmt.fmt.pix.width != cols) ||
                (fmt.fmt.pix.height != rows)) {
            fprintf (stderr, " format %s unavailable ask (%d %d) get (%d %d)\n",
                     nice_names[i], cols, rows, fmt.fmt.pix.width, fmt.fmt.pix.height);
            continue;
        }
        // when we fix capture stuff to properly handle non-zero padding, take this out
        if(fmt.fmt.pix.bytesperline != cols)
        {
            fprintf (stderr, " format %s unavailable ask (%d %d) get non-zero padding (%d)\n",
                     nice_names[i], cols, rows, fmt.fmt.pix.bytesperline);
            continue;
        }
        fprintf(stderr, "got fmt %s (%d, %d)\n", nice_names[i], cols, rows);
        break;
    }
    if(i == FMTS)
        goto fatal;
    v4l_format = formats[i];
    switch(formats[i])
    {
    case V4L2_PIX_FMT_YUV420:
        cap_format_out = AQ_YUV12;
        break;
    case V4L2_PIX_FMT_YUV410:
        cap_format_out = AQ_YUV9;
        break;
    case V4L2_PIX_FMT_RGB24:
        cap_format_out = AQ_RGB24;
        break;
    case V4L2_PIX_FMT_UYVY:
        cap_format_out = AQ_UYVY;
        break;
    case V4L2_PIX_FMT_YUYV:
        cap_format_out = AQ_YUY2;
        break;
    default:
        goto fatal;
    }

    name_out = DwString((const char *)cap.card);
    name_out += " ";
    name_out += dev;
    close(fd);
    return 1;
fatal:
    ;
    close(fd);
    return 0;
}

static
DwVec<DwString>
VideoDevices()
{
    DwString a;
    DwVec<DwString> ret;
    for(int i = 0; i < 4; ++i)
    {
        a = DwString("/dev/video%1").arg(DwString::fromInt(i));

        int f = open(a.c_str(), O_RDWR);
        if(f != -1)
        {
            ret.append(a);
            close(f);
        }
    }
    return ret;
}

static int
video_stream(int on)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;

    ret = ioctl (V4L_grabber, on ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type);
    if (ret < 0) {
        fprintf (stderr, "Unable to %s stream: %d.\n", on ? "start" : "stop", errno);
        return ret;
    }

    return 0;
}

static
char *
safer_copy(const DwString& b)
{
    size_t n = b.length();
    char *r = new char[n + 1];
    strncpy(r, b.c_str(), n + 1);
    // supposedly strncpy copies extra 0's
    // but this is just a safety thing.
    r[n] = 0;
    return r;
}

char **
DWYCOEXPORT
vgget_video_devices()
{
    // for now, assume we can do 160x120 and 320x240 on
    // all devices.

    DwVec<DwString> devs = VideoDevices();

    int t = devs.num_elems();
    int k = 0;
    char **r = new char *[4 * t + 1];
    for(int i = 0; i < t; ++i)
    {
        DwString name;
        int cap_format;
        int v4l_format;
        struct v4l_setup vs;
        if(probe_v4l(devs[i].c_str(), 320, 240, name, cap_format, v4l_format))
        {
            vs.name = name;
            vs.cap_format = cap_format;
            vs.cols = 320;
            vs.rows = 240;
            vs.dev = devs[i];
            vs.vidstd = "NTSC";
            vs.v4l_format = v4l_format;
            Devs.append(vs);
        } else if(probe_v4l(devs[i].c_str(),  352, 240, name, cap_format, v4l_format))
        {
            vs.name = name;
            vs.cap_format = cap_format;
            vs.cols = 352;
            vs.rows = 240;
            vs.dev = devs[i];
            vs.vidstd = "NTSC";
            vs.v4l_format = v4l_format;
            Devs.append(vs);
        }

        if(vs.name.length() > 0)
        {
            DwString b = "Large (~320x240) ";
            b += vs.name;
            r[k] = safer_copy(b);
            ++k;
        }

        vs.name = "";
        if(probe_v4l(devs[i].c_str(), 160, 120, name, cap_format, v4l_format))
        {
            vs.name = name;
            vs.cap_format = cap_format;
            vs.cols = 160;
            vs.rows = 120;
            vs.dev = devs[i];
            vs.vidstd = "NTSC";
            vs.v4l_format = v4l_format;
            Devs.append(vs);
        } else if(probe_v4l(devs[i].c_str(), 176, 120, name, cap_format, v4l_format))
        {
            vs.name = name;
            vs.cap_format = cap_format;
            vs.cols = 176;
            vs.rows = 120;
            vs.dev = devs[i];
            vs.vidstd = "NTSC";
            vs.v4l_format = v4l_format;
            Devs.append(vs);
        }
        if(vs.name.length() > 0)
        {
            DwString b = "Small (~160x120) ";
            b += vs.name;
            r[k] = safer_copy(b);
            ++k;
        }

        vs.name = "";
        if(probe_v4l(devs[i].c_str(), 640, 480, name, cap_format, v4l_format))
        {
            vs.name = name;
            vs.cap_format = cap_format;
            vs.cols = 640;
            vs.rows = 480;
            vs.dev = devs[i];
            vs.vidstd = "NTSC";
            vs.v4l_format = v4l_format;
            Devs.append(vs);
        }
        if(vs.name.length() > 0)
        {
            DwString b = "Huge (~640x480) ";
            b += vs.name;
            r[k] = safer_copy(b);
            ++k;
        }

        vs.name = "";
        if(probe_v4l(devs[i].c_str(), 1280, 720, name, cap_format, v4l_format))
        {
            vs.name = name;
            vs.cap_format = cap_format;
            vs.cols = 1280;
            vs.rows = 720;
            vs.dev = devs[i];
            vs.vidstd = "NTSC";
            vs.v4l_format = v4l_format;
            Devs.append(vs);
        }
        if(vs.name.length() > 0)
        {
            DwString b = "HD 720 (~1280x720) ";
            b += vs.name;
            r[k] = safer_copy(b);
            ++k;
        }
    }
    r[k] = 0;
    return r;

}

void
DWYCOEXPORT
vgfree_video_devices(char **d)
{
    char **tmp = d;
    while(*d)
    {
        delete [] *d;
        ++d;
    }
    delete [] tmp;
}

// if the video cap device is in use, we
// turn off the old one, select the new one, and
// restart capturing.
void
DWYCOEXPORT
vgset_video_device(int idx)
{
    if(idx < 0 || idx >= Devs.num_elems())
        return;
    if(V4L_grabber != -1)
    {
        vgstop_video_device();
    }
    Cur_dev = idx;
    if(!vginit(0, 0))
    {

    }
}

// this should shutdown and release the device.
void
DWYCOEXPORT
vgstop_video_device()
{
    if(V4L_grabber != -1)
    {
        vgstop(0);
        vgpass(0);
        close(V4L_grabber);
        V4L_grabber = -1;
    }
}

void
DWYCOEXPORT
vgshow_source_dialog()
{
}


void DWYCOEXPORT
vg_set_appdata(void *u1)
{
}

void
DWYCOEXPORT
vgnew(void *aqext)
{

}

void
DWYCOEXPORT
vgdel(void *aqext)
{
    if(V4L_grabber != -1)
    {
        vgstop(0);
        vgpass(0);
        close(V4L_grabber);
        V4L_grabber = -1;
    }


}

int
DWYCOEXPORT
vginit(void *aqext, int frame_rate)
{
    if(V4L_grabber != -1)
        return 1;

    int ret = 0;
    int fd;

    if ((fd = open (Devs[Cur_dev].dev.c_str(), O_RDWR)) == -1) {
        perror ("ERROR opening V4L interface \n");
        return 0;
    }

    int i;
    struct v4l2_format fmt;
    memset (&fmt, 0, sizeof (fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = Devs[Cur_dev].cols;
    fmt.fmt.pix.height = Devs[Cur_dev].rows;
    fmt.fmt.pix.pixelformat = Devs[Cur_dev].v4l_format;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    ret = ioctl (fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        goto fatal;
    }

    struct v4l2_requestbuffers rb;

    memset (&rb, 0, sizeof (rb));
    rb.count = NB_BUFFER;
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;

    ret = ioctl (fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
        fprintf (stderr, "Unable to allocate buffers: %d.\n", errno);
        goto fatal;
    }
    /* map the buffers */
    for (i = 0; i < NB_BUFFER; i++)
    {
        struct v4l2_buffer buf;

        memset (&buf, 0, sizeof (buf));
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl (fd, VIDIOC_QUERYBUF, &buf);
        if (ret < 0) {
            fprintf (stderr, "Unable to query buffer (%d).\n", errno);
            goto fatal;
        }
        if (debug)
            fprintf (stderr, "length: %u offset: %u\n", buf.length,
                     buf.m.offset);
        mem[i] = (unsigned char *)mmap (0 /* start anywhere */ ,
                                        buf.length, PROT_READ, MAP_SHARED, fd,
                                        buf.m.offset);
        if (mem[i] == MAP_FAILED) {
            fprintf (stderr, "Unable to map buffer (%d)\n", errno);
            goto fatal;
        }
        if (debug)
            fprintf (stderr, "Buffer mapped at address %p.\n", mem[i]);
    }
    /* Queue the buffers. */
    for (i = 0; i < NB_BUFFER; ++i) {
        struct v4l2_buffer buf;
        memset (&buf, 0, sizeof (buf));
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl (fd, VIDIOC_QBUF, &buf);
        if (ret < 0) {
            fprintf (stderr, "Unable to queue buffer (%d).\n", errno);
            goto fatal;
        }
    }

    struct v4l2_streamparm sparm;
    memset(&sparm, 0, sizeof(sparm));
    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_G_PARM, &sparm) != -1)
    {
        if(sparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
        {
            // adjust it to a default of 20fps since we don't really need
            // 30fps
            memset(&sparm, 0, sizeof(sparm));
            sparm.parm.capture.timeperframe.numerator = 1;
            sparm.parm.capture.timeperframe.denominator = 20;
            if(ioctl(fd, VIDIOC_S_PARM, &sparm) == -1)
            {
                fprintf (stderr, "Unable to set framerate (%d).\n", errno);
            }
        }

    }


    Selected_format = Devs[Cur_dev].cap_format;
    V4L_grabber = fd;

    pthread_mutex_init(&buf_mutex, 0);
    pthread_create(&v4l_thread, 0, grab_frames, 0);
    return 1;
fatal:
    ;
    close(fd);
    return 0;
}

int
DWYCOEXPORT
vghas_data(void *aqext)
{
    pthread_mutex_lock(&buf_mutex);
    if(next_buf == next_ibuf)
    {
        pthread_mutex_unlock(&buf_mutex);
        return 0;
    }
    pthread_mutex_unlock(&buf_mutex);
    return 1;
}

void
DWYCOEXPORT
vgneed(void *aqext)
{
}

void
DWYCOEXPORT
vgpass(void *aqext)
{
    pthread_mutex_lock(&buf_mutex);
    while(next_buf != next_ibuf)
    {
        delete [] bufs[next_buf];
        bufs[next_buf] = 0;
        //GRTLOG("chuck %d", (int)y_bufs[next_buf], 0);
        next_buf = (next_buf + 1) % bufs.num_elems();
    }
    pthread_mutex_unlock(&buf_mutex);
}

void
DWYCOEXPORT
vgstop(void *aqext)
{
    // XXX need to make sure thread is dead and then
    // see about the capture device too
    if(V4L_grabber == -1 || !v4l_thread)
        return;
    capexit = 1;
    pthread_join(v4l_thread, 0);
    v4l_thread = 0;
    capexit = 0;
}

void *
DWYCOEXPORT
vgget_data(
    void *aqext,
    int *c_out, int *r_out,
    int *bytes_out, int *fmt_out, unsigned long *captime_out)
{
    pthread_mutex_lock(&buf_mutex);
    if(next_buf != next_ibuf)
    {
        int nb = next_buf;
        void *bm = bufs[nb];

        *r_out = Devs[Cur_dev].rows;
        *c_out = Devs[Cur_dev].cols;
        *captime_out = y_bufs[nb];
        *fmt_out = (AQ_COLOR|Selected_format);
        *bytes_out = lens[nb];
        // dunno if i really have to repack like this, but
        // we'll do it anyway just in case.
#if 0
        int bytes = bm->Height * bm->Width * 3;
        unsigned char *tmp = new unsigned char[bytes];
        *bytes_out = bytes;
        for(int i = 0; i < bm->Height; ++i)
        {
            memcpy(&tmp[i*bm->Width*3], bm->ScanLine[i], bm->Width * 3);
        }
        delete bufs[nb];
        bm = 0;
#endif
        unsigned char *tmp = bufs[nb];
        bufs[nb] = 0;
        next_buf = (next_buf + 1) % bufs.num_elems();
        //GRTLOG("conv %d", (int)y_bufs[nb], 0);
        pthread_mutex_unlock(&buf_mutex);
        return tmp;
    }
    pthread_mutex_unlock(&buf_mutex);
    //oopanic("aqvfw get no data");
    // not reached
    return 0;
}

void
DWYCOEXPORT
vgfree_data(void *data)
{
    delete [] (unsigned char *)data;
}

void
DWYCOEXPORT
vgpreview_on(void *display_window)
{
}

void
DWYCOEXPORT
vgpreview_off()
{
}


#if 0
void
init_ext_vid()
{
    dwyco_set_external_video_capture_callbacks(
        vgnew,
        vgdel,
        vginit,
        vghas_data,
        vgneed,
        vgpass,
        vgstop,
        vgget_data
    );
}


#endif

static void *
grab_frames(void *)
{
    video_stream(1);
    while(!capexit)
    {
        // note: no need for this, we just continuously
        // capture over, since we want to drop frames
        // that aren't picked up in time.

        pthread_mutex_lock(&buf_mutex);
        if(next_buf == (next_ibuf + 1) % bufs.num_elems())
        {
            static struct timespec t = {0, 100L * 1000L};
            pthread_mutex_unlock(&buf_mutex);
            nanosleep(&t, 0);
            continue;
        }
        pthread_mutex_unlock(&buf_mutex);
        struct timeval tm;
        int len_out;

        struct v4l2_buffer buf;
        memset (&buf, 0, sizeof (struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        int ret = ioctl (V4L_grabber, VIDIOC_DQBUF, &buf);
        if (ret < 0) {
            fprintf (stderr, "Unable to dequeue buffer (%d).\n", errno);
            goto die;
        }

        len_out = buf.bytesused;
        unsigned char *new_buf = new unsigned char[len_out];
        memcpy(new_buf, mem[buf.index], len_out);
        // hmmm, the times returned by the drivers in the usec field appear
        // to be going up and down randomly. not sure if this is a driver
        // problem, or i just misunderstand the docs or what... there seems
        // to be some confusion in the docs about the type of this field
        // anyways, but the hack below appears to fix it.
        if(0 && (buf.flags & V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC))
        {
            tm = buf.timestamp;
            //fprintf (stderr, "cap (%lu, %lu).\n", tm.tv_sec, tm.tv_usec);
        }
        else
        {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            tm.tv_sec = ts.tv_sec;
            tm.tv_usec = ts.tv_nsec / 1000;
            //fprintf (stderr, "mono (%lu, %lu).\n", tm.tv_sec, tm.tv_usec);
        }


        ret = ioctl (V4L_grabber, VIDIOC_QBUF, &buf);
        if (ret < 0) {
            fprintf (stderr, "Unable to requeue buffer (%d).\n", errno);
            delete [] new_buf;
            goto die;
        }


        pthread_mutex_lock(&buf_mutex);
        bufs[next_ibuf] = new_buf;
        y_bufs[next_ibuf] = ((tm.tv_sec * 1000000) + tm.tv_usec) / 1000; // turn into msecs
        lens[next_ibuf] = len_out;
        next_ibuf = (next_ibuf + 1) % bufs.num_elems();
        pthread_mutex_unlock(&buf_mutex);
    }
die:
    ;
    video_stream(0);
    return 0;
}
