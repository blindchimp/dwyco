
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// note: this is a proof of concept driver that uses qt5 qtmultimedia
// to capture frames. as usual, there are a lot of gotchas:
//
// * in a qwidgets app, this is your *only* option for MacOS. there is
//      no "native" driver for MacOS. This driver *might* work on other
//      platforms, but the native drivers for windows and linux work much
//      better. USE_QML_CAMERA must be undefined. on MacOS,
//      DWYCO_FORCE_DESKTOP_VGQT must be defined. otherwise leave it undefined
//      to use the native drivers.
//
// * in a QML app, you have some options:
//      if you want to manipulate the Camera object using QML, but use this driver to
//      capture video frames, you MUST put the following in the QML Camera object
//      Camera { objectName: "qrCameraQML" ....}
//      so this driver can find the camera object. you must also define USE_QML_CAMERA
//      when compiling this file. this is your only option for camera capture on Android
//      since qtmultimediawidgets is not supported on android.
//
//      for desktop QML apps, you *may* be able to get away with using this driver without
//      USE_QML_CAMERA defined. i haven't tested it lately, but as long as you are not
//      defining the camera object in QML, this driver might be able to enumerate and
//      offer capture services for the camera in whatever default state it comes up in.
//      webcams don't provide a lot of special configuration (unlike mobile cameras) so
//      this might work ok for desktop.
//
// for testing, if you define TEST_THREAD, this driver creates a thread that will produce
// video test frames without accessing a camera device. this is very useful if your video
// device driver is fussy or crashes your computer during debugging.
//



#include <QList>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QCamera>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QVideoSink>
#include <QtConcurrent>
#include <QFuture>
#include <QThread>
#include <QDebug>
#include <stdlib.h>
#include "vgqt.h"
#include "vidaq.h"
#include "pgm.h"
#include "vidcvt.h"

#include <stdio.h>
#include <time.h>
#ifdef _Windows
#include <WinSock2.h>
#endif
#ifdef LINUX
#include <unistd.h>
#include <string.h>
#endif

//#define USE_QML_CAMERA

#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#define STBIR_DEFAULT_FILTER_DOWNSAMPLE   STBIR_FILTER_BOX
#define STBIR_SATURATE_INT
#include "stb_image_resize.h"
#undef TEST_THREAD
#ifdef TEST_THREAD
#include <pthread.h>
#endif

#ifdef USE_QML_CAMERA
#include <QQmlApplicationEngine>
extern QQmlApplicationEngine *TheEngine;
#endif

#define NB_BUFFER 3
//static QVector<unsigned long> y_bufs(NB_BUFFER);
//static QVector<unsigned int> lens(NB_BUFFER);
//static QVector<QVideoFrame> vbufs(NB_BUFFER);
struct vframe
{
    vframe() {captime = 0;}
    QVideoFrame frm;
    unsigned long captime;
};

static struct vframe Raw_frame;

//static int next_ibuf;
//static int next_buf;
static int debug = 1;
static QMutex mutex;
static int Orientation;
static QCamera *Cam;
static QMediaCaptureSession *Mcs;
static QMetaObject::Connection Cam_sig;
static QList<QCameraDevice> Cams;
static QList<QCameraFormat> CFormats;
static int Cur_idx = -1;
QDebug operator<<(QDebug dbg, const QCameraFormat &type);
int VGQT_swap_rb;
int VGQT_flip;

struct finished
{
    finished() {
        r = c = bytes = fmt = -1;
        planes[0] = 0;
        planes[1] = 0;
        planes[2] = 0;
        captime = 0;
    }

    void flush() {
        pgm_freearray(planes[0], r);
        pgm_freearray(planes[1], r / 2);
        pgm_freearray(planes[2], r / 2);
    }

    gray **planes[3];
    int r;
    int c;
    int bytes;
    int fmt;
    unsigned long captime;
};
static QVector<QFuture<finished> > futs(NB_BUFFER);
//static QVector<finished> conv_buf(NB_BUFFER);
static int next_cb;
static int next_icb;

void oopanic(const char *);


//#include "vgqt.moc"

static int stop_thread;
#ifdef TEST_THREAD
static pthread_t thread;
#endif

// this is needed because on older versions of
// ios (like 9), it doesn't have clock_gettime, and
// in addition to that, it weak-links it, so the program
// just crashes despite compiling and deploying ok.
//
#if defined(DWYCO_IOS) || defined(MACOSX)
#include <sys/time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
static int
clock_gettime(int, struct timespec *t)
{
    struct timeval now;
    int rv = gettimeofday(&now, 0);
    if(rv) return rv;
    t->tv_sec = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#endif

static
void
stop_all_conversions()
{
    for(int i = 0; i < NB_BUFFER; ++i)
    {
        // note: we can't cancel the future, but we
        // did set it to an empty future, which the docs
        // is "canceled"
        if(!futs[i].isCanceled())
        {
            finished f = futs[i].result();
            f.flush();
            futs[i] = QFuture<finished>();
        }
    }
    next_cb = 0;
    next_icb = 0;
}

#ifdef TEST_THREAD
static void
add_frame(QVideoFrame frm)
{
    QMutexLocker ml(&mutex);

    if(next_buf == (next_ibuf + 1) % NB_BUFFER)
    {
        // drop it for now, maybe something more complicated
        // like overwriting next frame would look better
        // in some cases, but not worth it at this point.
        return;
    }
    struct timeval tm;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tm.tv_sec = ts.tv_sec;
    tm.tv_usec = ts.tv_nsec / 1000;

    vbufs[next_ibuf] = frm;
    y_bufs[next_ibuf] = ((tm.tv_sec * 1000000) + tm.tv_usec) / 1000; // turn into msecs
    next_ibuf = (next_ibuf + 1) % NB_BUFFER;
}

static void *
test_thread(void *)
{
    int t = 0;
    Orientation = 270;
    while(1)
    {
        if(stop_thread)
            return 0;
        QVideoFrame f(320*240+2*(160 * 120), QSize(320, 240), 320, QVideoFrame::Format_NV21);
        f.map(QAbstractVideoBuffer::ReadWrite);
        uchar *bits = f.bits();
        memset(bits, t, 320 * 240);
        memset(bits + 320*60, 0, 120);
        memset(bits + 320*61, 0, 120);
        memset(bits + 320*62, 255, 120);
        memset(bits + 320*63, 255, 120);
        memset(bits + 320*240, 0, 2 * 160 * 120);
        ++t;
        f.unmap();
        add_frame(f);
        QThread::msleep(30);
    }
    return 0;
}

char **
DWYCOEXPORT
vgqt_get_video_devices()
{
    char **r = new char *[2];
    r[0] = new char [sizeof("Synth") + 1];
    strcpy(r[0], "Synth");
    r[1] = 0;
    return r;
}

void
DWYCOEXPORT
vgqt_free_video_devices(char **d)
{
    char **tmp = d;
    while(*d)
    {
        delete [] *d;
        ++d;
    }
    delete [] tmp;
}

#endif

static
gray **
pgm_rot(gray **inp, int& cols, int& rows, int rot)
{
    if(rot == 0)
        return inp;
    gray *s = inp[0]; //&inp[0][0];
    gray **dst;
    if(rot == 180)
    {
        dst = pgm_allocarray(cols, rows);
        for(int r = 0; r < rows; ++r)
        {
            for(int c = 0; c < cols; ++c)
            {
                dst[rows - 1 - r][cols - 1 - c] = *s++;
            }
        }
    }
    else
    {
        int tmp = cols;
        cols = rows;
        rows = tmp;
        dst = pgm_allocarray(cols, rows);
        if(rot == 90)
        {
            for(int c = cols - 1; c >= 0; --c)
            {
                for(int r = 0; r < rows; ++r)
                {
                    dst[r][c] = *s++;
                }
            }
        }
        else if(rot == 270)
        {
            for(int c = 0; c < cols; ++c)
            {
                for(int r = rows - 1; r >= 0; --r)
                {
                    dst[r][c] = *s++;
                }
            }

        }
    }
    return dst;

}

// this is for nv21
// c should point to the first sample in the block of
// interleaved chroma samples
static void
get_interleaved_chroma_planes(int ccols, int crows, unsigned char *c, gray**& vu_out, gray**& vv_out, int subsample)
{
    int ch_cols = ccols / subsample;
    int ch_rows = crows / subsample;
    int autoconfig = 0;
#ifdef MACOSX
    int upside_down = 1;
#else
    int upside_down = 0;
#endif

    gray **u_out = pgm_allocarray(ch_cols, ch_rows);
    gray **v_out = pgm_allocarray(ch_cols, ch_rows);
    vu_out = u_out;
    vv_out = v_out;

    if(!upside_down)
    {
        gray *ut = &u_out[0][0];
        gray *vt = &v_out[0][0];
        gray *us = c;
        gray *vs = c + 1;
        for(int i = 0; i < ch_rows; ++i)
        {
            for(int j = 0; j < ch_cols; ++j)
            {
                *ut++ = *us;
                us += 2;
                *vt++ = *vs;
                vs += 2;
            }
        }
    }
    else
    {
        gray *us = c;
        gray *vs = c + 1;
        for(int i = ch_rows - 1; i >= 0; --i)
        {
            gray *ut = &u_out[i][0];
            gray *vt = &v_out[i][0];
            for(int j = 0; j < ch_cols; ++j)
            {
                *ut++ = *us;
                us += 2;
                *vt++ = *vs;
                vs += 2;
            }
        }
    }
}

QDebug operator<<(QDebug dbg, const QCameraFormat &format)
{
    // Get the individual format settings

    const int resolutionWidth = format.resolution().width();
    const int resolutionHeight = format.resolution().height();
    const QVideoFrameFormat::PixelFormat pixelFormat = format.pixelFormat();


    // Output the format settings to qdebug

    dbg << "(" << resolutionWidth << "x" << resolutionHeight << ",";
    dbg << pixelFormat << " " << format.maxFrameRate() << ")";
    return dbg;

}

#ifndef TEST_THREAD
static
QCameraFormat
find_closest(const QList<QCameraFormat>& formats, int cols, int rows)
{

    int fmts[] = {
        QVideoFrameFormat::Format_YUV420P,
        QVideoFrameFormat::Format_YV12,
        QVideoFrameFormat::Format_NV12,
        QVideoFrameFormat::Format_NV21,
        QVideoFrameFormat::Format_UYVY,
        QVideoFrameFormat::Format_YUYV,
        QVideoFrameFormat::Format_RGBX8888
    };
    int nf = sizeof(fmts) / sizeof(fmts[0]);

	int n = formats.count();
    int best = INT_MAX;
    QCameraFormat bestf;
    for(int i = 0; i < n; ++i)
    {
        auto f = formats[i];
        int a = abs(f.resolution().height() - rows) + abs(f.resolution().width() - cols);
        if(a < best)
        {
            for(int j = 0; j < nf; ++j)
            {
                if(f.pixelFormat() == fmts[j])
                {
                    best = a;
                    bestf = f;
                    break;
                }
            }
        }
    }
    return bestf;
}

static
QByteArray
get_format(QCameraDevice dev, int cols, int rows, QCameraFormat& format_out)
{

    QByteArray desc;
    if(dev.isNull())
    {
        desc = "unavailable";
       format_out = QCameraFormat();
    }
    else
    {
        qDebug() << dev.videoFormats() << "\n";
        auto cf = find_closest(dev.videoFormats(), cols, rows);
        format_out = cf;
        //note: avoid situations where camera may have names
        //that include unicode names or whatever (like korean names).
        desc = "Camera"; //dev.description().toLatin1();
        if(cf.isNull())
        {
            desc = "unavailable";
        }
    }
    return desc;
}


char **
DWYCOEXPORT
vgqt_get_video_devices()
{
    Cams = QMediaDevices::videoInputs();
    int n = Cams.count();
    char **r = new char *[3 * n + 1];
    for(int i = 0; i < n * 3; i += 3)
    {
        QByteArray desc;
        QCameraFormat format;
        QCameraDevice dev = Cams[i / 3];
        QByteArray camnum = QByteArray::number((i / 3) + 1);
        desc = get_format(dev, 640, 480, format);
        desc += " #";
        desc += camnum;
        desc += " (Normal)";
        r[i] = new char [desc.length() + 1];
        strcpy(r[i], desc.constData());
        CFormats.append(format);
        qDebug() << desc << " " << format << "\n";

        desc = get_format(dev, 320, 240, format);
        desc += " #";
        desc += camnum;
        desc += " (Small)";
        r[i + 1] = new char [desc.length() + 1];
        strcpy(r[i + 1], desc.constData());
        CFormats.append(format);
         qDebug() << desc << " " << format << "\n";

        desc = get_format(dev, 1280, 960, format);
        desc += " #";
        desc += camnum;
        desc += " (Large)";
        r[i + 2] = new char [desc.length() + 1];
        strcpy(r[i + 2], desc.constData());
        CFormats.append(format);
         qDebug() << desc << " " << format << "\n";
    }
    r[3 * n] = 0;
    return r;
}

void
DWYCOEXPORT
vgqt_free_video_devices(char **d)
{
    char **tmp = d;
    while(*d)
    {
        delete [] *d;
        ++d;
    }
    delete [] tmp;
}
#endif

// if the video cap device is in use, we
// turn off the old one, select the new one, and
// restart capturing.
void
DWYCOEXPORT
vgqt_set_video_device(int idx)
{
    vgqt_stop_video_device();

    Cur_idx = idx;

    if(!vgqt_init(0, 0))
    {

    }
}

void
DWYCOEXPORT
vgqt_show_source_dialog()
{
}


void DWYCOEXPORT
vgqt_set_appdata(void *u1)
{
}

void
DWYCOEXPORT
vgqt_preview_on(void *display_window)
{
}

void
DWYCOEXPORT
vgqt_preview_off()
{
}

// this should shutdown and release the device.
void
DWYCOEXPORT
vgqt_stop_video_device()
{
    if(Mcs)
    {
        vgqt_stop(0);
        vgqt_pass(0);
    }
    stop_all_conversions();

#ifndef USE_QML_CAMERA
    if(Cam)
    {
        delete Cam;
        Cam = 0;
    }
#endif
}



void
DWYCOEXPORT
vgqt_new(void *aqext)
{
    if(Mcs)
        return;
    Mcs = new QMediaCaptureSession;
    Raw_frame = vframe();
#ifdef TEST_THREAD
    stop_thread = 0;
    pthread_create(&thread, 0, test_thread, 0);
#endif
}

void
DWYCOEXPORT
vgqt_del(void *aqext)
{

    if(Mcs)
    {
        vgqt_stop(0);
        vgqt_pass(0);
        delete Mcs;
        Mcs = 0;
    }
#ifndef USE_QML_CAMERA
    if(Cam)
    {
        delete Cam;
        Cam = 0;
    }
#endif

    stop_thread = 1;
    QMutexLocker ml(&mutex);
    stop_all_conversions();
    Raw_frame = vframe();
#ifdef TEST_THREAD
    pthread_join(thread, 0);
#endif
}

#ifdef USE_QML_CAMERA
static
QCamera *
find_qml_camera()
{
    QList<QObject *> ro = TheEngine->rootObjects();
    for(int i = 0; i < ro.count(); ++i)
    {
        QObject *qmlCamera = ro[i]->findChild<QObject*>("qrCameraQML");
        if(!qmlCamera)
        {
            continue;
        }
        QCamera *camera_ = qvariant_cast<QCamera*>(qmlCamera->property("mediaObject"));
        if(!camera_)
            return 0;
        return camera_;
    }
    return 0;
}
#endif

static
void
config_viewfinder(bool state)
{

}
#if 0
static
void
config_viewfinder(QCamera::State state)
{
    if(state != QCamera::LoadedState)
        return;
    vidsurf *vf = new vidsurf;
    //vf->show();
    Cam->setViewfinder(vf);
    QCameraViewfinderSettings vfs;
    vfs = Cam->viewfinderSettings();
    vfs.setPixelFormat(QVideoFrame::Format_NV12);
    vfs.setResolution(640, 480);
    Cam->setViewfinderSettings(vfs);
    qDebug() << "VFS" << Cam->viewfinderSettings().pixelFormat() << "\n";
    Cam->start();

}
#endif

static
void
new_video_frame(const QVideoFrame& frm)
{
    //qDebug() << frm.pixelFormat() << "\n";
    Raw_frame.frm = frm;
#ifdef __WIN32__
    Raw_frame.captime = timeGetTime();
#else
    struct timeval tm;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tm.tv_sec = ts.tv_sec;
    tm.tv_usec = ts.tv_nsec / 1000;
    Raw_frame.captime = ((tm.tv_sec * 1000000) + tm.tv_usec) / 1000; // turn into msecs
#endif
}
int
DWYCOEXPORT
vgqt_init(void *aqext, int frame_rate)
{

#ifdef TEST_THREAD
    if(thread == 0)
    {
        stop_thread = 0;
        pthread_create(&thread, 0, test_thread, 0);
    }
    return 1;
#endif
    if(!Mcs)
        return 0;
#ifdef USE_QML_CAMERA
    Cam = find_qml_camera();
    if(!Cam)
        return 0;
#endif

    if(!Cam)
    {
        if(Cur_idx < 0 || Cur_idx >= Cams.count() * 3)
            return 0;
        Cam = new QCamera(Cams[Cur_idx / 3]);
        Cam->setCameraFormat(CFormats[Cur_idx]);
        QObject::connect(Cam, &QCamera::activeChanged, config_viewfinder);
        QVideoSink *vs = new QVideoSink;
        Mcs->setCamera(Cam);
        Mcs->setVideoSink(vs);
        QObject::connect(vs, &QVideoSink::videoFrameChanged, new_video_frame);
    }

    Cam->start();
    return 1;

#if 0 && (defined(DWYCO_IOS) || defined(MACOSX))
    QCameraViewfinderSettings vfs;
    vfs = Cam->viewfinderSettings();
    vfs.setPixelFormat(QVideoFrame::Format_NV12);
    vfs.setResolution(640, 480);
    Cam->setViewfinderSettings(vfs);
//#elif !defined(ANDROID)
    // NOTE: qt5.10.1 doesn't support videoprobe on windows
    // other platforms, not sure. rgb and 420p seem to be returned tho.
    // on linux, i think yv12 is returned, but not sure.
    // on qt5.11.1 the probe appears to work, but the format
    // isn't being set properly, so the video is garbled.
    // can't be bothered to debug it at this point, will just
    // stick with older mtcapxe stuff.

    QList<QVideoFrame::PixelFormat> pf = Cam->supportedViewfinderPixelFormats();
    if(pf.count() == 0)
    {
        qDebug() << "NO FORMATS\n";
        return 0;
    }
    qDebug() << "FORMATS\n";
    for(int i = 0; i < pf.count(); ++i)
    {
        qDebug() << pf[i] << "\n";
    }
    qDebug() << "DONE\n";
    QList<QSize> sz = Cam->supportedViewfinderResolutions();
    qDebug() << sz << "\n";
#endif

    //Orientation = caminfo.orientation();
    //QObject::connect(&Probe_handler->probe, SIGNAL(videoFrameProbed(QVideoFrame)),
    //                 Probe_handler, SLOT(handleFrame(QVideoFrame)), Qt::UniqueConnection);


    return 0;

}

int
DWYCOEXPORT
vgqt_has_data(void *aqext)
{
    QMutexLocker ml(&mutex);

    if(next_cb == next_icb)
    {
        return 0;
    }
    if(futs[next_icb].isFinished())
        return 1;
    return 0;
}

void
DWYCOEXPORT
vgqt_pass(void *aqext)
{
    QMutexLocker ml(&mutex);
    Raw_frame = vframe();
}

void
DWYCOEXPORT
vgqt_stop(void *aqext)
{
    // XXX need to make sure thread is dead and then
    // see about the capture device too
    if(!Mcs)
        return;

    stop_all_conversions();
}

void *
DWYCOEXPORT
vgqt_get_data(
    void *aqext,
    int *c_out, int *r_out,
    int *bytes_out, int *fmt_out, unsigned long *captime_out)
{
    QMutexLocker ml(&mutex);

    finished *f;
    if(!futs[next_icb].isFinished())
        oopanic("bad get data");
    f = new finished;
    *f = futs[next_icb].result();
    // note kludge, so on shutdown, we can
    // distinguish between unused futures and ones
    // that might still be running, so we can
    // flush them to avoid leaks
    futs[next_icb] = QFuture<finished>();
    *c_out = f->c;
    *r_out = f->r;
    *bytes_out = f->bytes;
    *fmt_out = f->fmt;
    *captime_out = f->captime;

    next_icb = (next_icb + 1) % NB_BUFFER;

    // this is ugly, should fix this, but can't be bothered
    // until some testing is done
    return f;
}

template<class T>
void
flip_in_place(T **img, int cols, int rows)
{
    T *tmp = (T *)pm_allocrow(cols, sizeof(T));
    int lim = rows / 2;

    for(int i = 0; i < lim; ++i)
    {
        bcopy(&img[i][0], &tmp[0], cols * sizeof(T));
        bcopy(&img[rows - i - 1][0], &img[i][0], cols * sizeof(T));
        bcopy(&tmp[0], &img[rows - i - 1][0], cols * sizeof(T));
    }
    pm_freerow((char *)tmp);
}

static
struct finished
conv_data(vframe ivf)
{
    struct finished f;


        int cols, rows;
        QVideoFrame vf = ivf.frm;
        f.captime = ivf.captime;

        if(!vf.map(QVideoFrame::ReadOnly))
        {
            oopanic("can't map");
        }
        f.r = rows = vf.height();
        f.c = cols = vf.width();
        int flipped = vf.surfaceFormat().scanLineDirection() == QVideoFrameFormat::TopToBottom;

        int fmt = 0;
        int swap = 0;
        QVideoFrameFormat::PixelFormat vfpf = vf.pixelFormat();
        switch(vfpf)
        {
        case QVideoFrameFormat::Format_RGBX8888:
            // XXX FIX ME, this won't work since there are no video frame formats
            // that use 24bit rgb formats in qt6. so we would need to update
            // our converter if we ever came across a camera that produces this
            fmt = AQ_RGB24;
            break;
        case QVideoFrameFormat::Format_YUV420P:
            fmt = AQ_YUV12;
            //swap = 1;
            break;
        case QVideoFrameFormat::Format_YV12:
            fmt = AQ_YUV12;
            break;
        case QVideoFrameFormat::Format_UYVY:
            fmt = AQ_UYVY;
            break;
        case QVideoFrameFormat::Format_YUYV:
            fmt = AQ_YUY2;
            break;
        // note: NV12 seems to be the closest thing to
        // NV21 provided by the qt ios driver. this isn't
        // in the documentation, but was gleaned from reading
        // the source for 5.6.2. it *appears* to work, as long
        // as you set the format explicitly in the camera setup.
        // though it needs more testing because sometimes it doesn't
        // appear to get setup properly, and we still end up with
        // ARGB32 in here.
        case QVideoFrameFormat::Format_NV12:
            swap = 1;
        // FALL THRU
        case QVideoFrameFormat::Format_NV21:
            fmt = AQ_NV21;
            // for now, if the stride isn't the same as
            // the dimensions, bail
            if(vf.bytesPerLine(0) != vf.width())
                ::abort();
            break;
        default:
            // just a guess so we don't crash
            fmt = AQ_RGB24;
        }

        // note for android, nv21 is standard, and we convert to
        // yuv12
        f.fmt = (AQ_COLOR|AQ_YUV12);
#if !(defined(ANDROID) || defined(MACOSX))
        if(fmt & AQ_YUV12)
        {
            unsigned char *c = (unsigned char *)vf.bits(0);
            gray **g = pgm_allocarray(cols, rows);
            memcpy(&g[0][0], c, cols * rows);
            if(flipped ^ VGQT_flip)
                flip_in_place(g, cols, rows);
            //c += cols * rows;
            f.planes[0] = g;

            c = vf.bits(1);
            g = pgm_allocarray(cols / 2, rows / 2);
            memcpy(&g[0][0], c, (cols * rows) / 4);
            if(flipped ^ VGQT_flip)
                flip_in_place(g, cols / 2, rows / 2);
            f.planes[1] = g;
            //c += (cols * rows) / 4;

            c = vf.bits(2);
            g = pgm_allocarray(cols / 2, rows / 2);
            memcpy(&g[0][0], c, (cols * rows) / 4);
            if(flipped ^ VGQT_flip)
                flip_in_place(g, cols / 2, rows / 2);
            f.planes[2] = g;

            if(swap ^ VGQT_swap_rb)
            {
                gray **tmp = f.planes[1];
                f.planes[1] = f.planes[2];
                f.planes[2] = tmp;
            }

            vf.unmap();
            vf = QVideoFrame();
            return f;
        }
        else
        {
            // note: this is assumed to be some kind of interleaved format with dense lines.
            // we'll need to update the converter to add a "line_length" or something if
            // we ever run into one of those.
            VidConvert cvt;
            cvt.set_style(fmt|AQ_COLOR);
            cvt.swap_uv = swap ^ VGQT_swap_rb;
            cvt.set_upside_down(!flipped ^ VGQT_flip);
            int ccols = vf.width();
            int crows = vf.height();
            void *y;
            void *cb;
            void *cr;
            int fmt_out;
            int total_size = 0;
            for(int i = 0; i < vf.planeCount(); ++i)
            {
                total_size += vf.mappedBytes(i);
            }
            void *r = cvt.convert_data(vf.bits(0), total_size, ccols, crows, y, cb, cr, fmt_out, 0);
            f.planes[0] = (gray **)r;
            f.planes[1] = (gray **)cb;
            f.planes[2] = (gray **)cr;

            vf.unmap();
            vf = QVideoFrame();
            return f;
        }

#else

        unsigned char *c = (unsigned char *)vf.bits(0);
#define SSCOLS (640)
#define SSROWS (calcrows)
        int calcrows = (float)rows / ((float)cols / SSCOLS);
        if(calcrows % 2 != 0) ++calcrows;

        gray **g = pgm_allocarray(SSCOLS, SSROWS);
        int ncols = SSCOLS;
        int nrows = SSROWS;
        if(cols == SSCOLS && rows == SSROWS)
            memcpy(&g[0][0], c, cols * rows);
        else
            stbir_resize_uint8(c, cols, rows, 0, &g[0][0], SSCOLS, SSROWS, 0, 1);

        // NOTE: this flipping is for cdc-x compatibility.
        // the driver produces flipped images because the old ms
        // drivers did that. naturally, we end up flipping things
        // again, so this is a total waste. someday, i'll just
        // force the ms driver to produce the right stuff and
        // get rid of mose of this other stuff. on android, i'll
        // have to revisit this, because in that case, there are
        // other orientation issues.
#ifdef MACOSX
        flip_in_place(g, ncols, nrows);
#endif

        //
        if(Orientation != 0)
        {
            gray **rg = pgm_rot(g, ncols, nrows, Orientation);
            pgm_freearray(g, SSROWS);
            g = rg;
        }

        c += f.c * f.r;
        c = vf.bits(1);
        // note: 2 channels, cb and cr
        gray **cr;
        gray **cb;

        if(cols == SSCOLS && rows == SSROWS)
        {
            get_interleaved_chroma_planes(SSCOLS / 2, SSROWS / 2, c, cr, cb, 1);
        }
        else
        {
            gray **gc = pgm_allocarray(SSCOLS, SSROWS / 2);
            stbir_resize_uint8(c, cols / 2, rows / 2, 0, &gc[0][0], SSCOLS / 2, SSROWS / 2, 0, 2);
            get_interleaved_chroma_planes(SSCOLS / 2, SSROWS / 2, &gc[0][0], cr, cb, 1);
            pgm_freearray(gc, SSROWS / 2);
        }

        if(swap)
        {
            gray **tmp = cr;
            cr = cb;
            cb = tmp;
        }


        if(Orientation != 0)
        {
            int c = SSCOLS / 2;
            int r = SSROWS / 2;
            gray **rcr = pgm_rot(cr, c, r, Orientation);
            pgm_freearray(cr, SSROWS / 2);
            cr = rcr;

            c = SSCOLS / 2;
            r = SSROWS / 2;
            gray **rcb = pgm_rot(cb, c, r, Orientation);
            pgm_freearray(cb, SSROWS / 2);
            cb = rcb;
        }

        //cols = SSCOLS;
        //rows = SSROWS;
        f.c = ncols;
        f.r = nrows;

        f.planes[0] = g;
        f.planes[1] = cb;
        f.planes[2] = cr;
        // note: f.bytes isn't really used
        // on android, and it doesn't make
        // a lot of sense since we are returning
        // discontiguous blocks of memory for the
        // planes.

        vf.unmap();
        vf = QVideoFrame();
        return f;
#endif
    oopanic("aqvfw get no data");
    // not reached
    return finished();
}

void
DWYCOEXPORT
vgqt_free_data(void *data)
{
    delete (finished *)data;
}

void
DWYCOEXPORT
vgqt_need(void *aqext)
{
    QMutexLocker ml(&mutex);
    if(!Raw_frame.frm.isValid())
        return;
    if(next_cb == (next_icb + 1) % NB_BUFFER)
    {
        // no room to start another conversion
        return;
    }
    futs[next_cb] = QtConcurrent::run(conv_data, Raw_frame);
    Raw_frame = vframe();
    next_cb = (next_cb + 1) % NB_BUFFER;
}



