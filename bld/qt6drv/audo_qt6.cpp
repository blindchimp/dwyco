
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QList>
#include <QAudioOutput>
#include <QRecursiveMutex>
#include <QMutexLocker>
#include <QGuiApplication>
#include <QObject>
#include <QIODevice>
#include <QTimer>
#include <QAudioSink>
#include <QMediaDevices>

#include "dlli.h"

//#include "audo.h"
//#include "audmix.h"
//#include "audth.h"
// this is a hack, we don't want to include internal stuff
// from the bowels, so we just define this internal API
// to do the funky calls for us.
void mixer_buf_done_callback();
#define GRTLOG(a,b,c) APP_GRTLOG(a,b,c)
#define GRTLOGA(a,b,c,d,e,f) APP_GRTLOGA(a,b,c,d,e,f)

// ack, 44.1khz * 80ms * 2 bytes
//#define AUDBUF_LEN (7056)
#if defined(ANDROID) || defined(DWYCO_IOS)
#define UWB_SAMPLE_RATE 8000
#else
#define UWB_SAMPLE_RATE 44100
#endif
#define AUDBUF_LEN ((int)(UWB_SAMPLE_RATE * .080 * 2))

struct audpack
{
    char *buf;
    int len;
    int pos;
    int user_data;
};
static QList<audpack> *devq_p;
static QList<audpack> *doneq;
static int Init;
static QRecursiveMutex audio_mutex;

class Audo_qio : public QIODevice
{
public:
    Audo_qio(QObject *p = 0) : QIODevice(p) {}

    qint64 readData(char *data, qint64 maxlen);

    qint64 writeData(const char *, qint64) {
        return 0;
    }

    qint64 bytesAvailable() const {
        int tot = 0;
        for(int i = 0; i < devq_p->count(); ++i)
        {
            tot += (*devq_p)[i].len;
        }
        return tot + QIODevice::bytesAvailable();
    }


};

struct mumble : public QObject
{
    Q_OBJECT
public:
    mumble() : QObject(0) {
        QObject::connect(this, &mumble::suspend, this, &mumble::dosuspend, Qt::QueuedConnection);
        QObject::connect(this, &mumble::reset, this, &mumble::doreset, Qt::QueuedConnection);
        QObject::connect(this, &mumble::init, this, &mumble::doinit, Qt::QueuedConnection);
        //qio_dev = new Audo_qio;
        //qio_dev->open(QIODevice::ReadOnly);
        qio_dev = 0;
        audio_output = 0;
        audio_poll_timer = new QTimer;
        audio_poll_timer->setTimerType(Qt::PreciseTimer);
        audio_poll_timer->setSingleShot(false);
        connect(audio_poll_timer, SIGNAL(timeout()), this, SLOT(qt_pushmore()));
        audio_poll_timer->start(10);
    }

    ~mumble() {
        //delete qio_dev;
        qio_dev = 0;
        delete audio_output;
        audio_output = 0;
        audio_poll_timer->deleteLater();
        audio_poll_timer = 0;
    }
    QIODevice *qio_dev;
    QAudioSink *audio_output;
    QTimer *audio_poll_timer;


public slots:
    void qt_pushmore();

    void dosuspend()
    {
        QMutexLocker ml(&audio_mutex);
        if(!audio_output)
            return;
        audio_output->suspend();
    }

    void doreset()
    {
        QMutexLocker ml(&audio_mutex);
        if(!audio_output)
            return;
        audio_output->reset();
    }

    void handle_stateChange(QAudio::State a) {
       QMutexLocker ml(&audio_mutex);
        GRTLOG("state change to %d", a, 0);
       if(a == QAudio::IdleState)
       {
           // if(audio_output)
           // {
           //     audio_output->start();
           // }
       }

    }

    void
    doinit()
    {
        QMutexLocker ml(&audio_mutex);
        if(audio_output)
        {
            if(audio_output->state() != QAudio::ActiveState)
            {
                qio_dev = audio_output->start();
                audio_output->setVolume(1.0);
            }
            return;
        }

        const auto devices = QMediaDevices::audioOutputs();
        for (const QAudioDevice &device : devices)
            qDebug() << "Device: " << device.description();

        QAudioFormat af;
        af.setSampleRate(UWB_SAMPLE_RATE);
        af.setSampleFormat(QAudioFormat::Int16);
        af.setChannelCount(1);
        af.setChannelConfig(QAudioFormat::ChannelConfigMono);

            QAudioDevice info(QMediaDevices::defaultAudioOutput());
            if (!info.isFormatSupported(af)) {
                qWarning() << "Raw audio format not supported by backend, cannot play audio.";
                return;
            }


        audio_output = new QAudioSink(af);
        audio_output->setBufferSize(3 * AUDBUF_LEN);
        connect(audio_output, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handle_stateChange(QAudio::State)));
        audio_output->setVolume(1.0);
        qio_dev = audio_output->start();

    }

signals:
    void suspend();
    void reset();
    void init();

};
#include "audo_qt6.moc"

static mumble *m;

static
int
qt_filler(void *userdata, char *buf, int len)
{
    QList<audpack>& devq = *devq_p;

    GRTLOGA("filler devq %d doneq %d len %d", devq.count(), doneq->count(), len, 0, 0);

    int toload = len;
    while(toload > 0 && devq.count() > 0)
    {
        if(toload < devq[0].len - devq[0].pos)
        {
            memcpy(buf, devq[0].buf + devq[0].pos, toload);
            devq[0].pos += toload;
            toload = 0;
            buf += toload;
            break;
        }
        else if(toload >= devq[0].len - devq[0].pos)
        {
            memcpy(buf, devq[0].buf + devq[0].pos, devq[0].len - devq[0].pos);
            buf += devq[0].len - devq[0].pos;
            toload -= devq[0].len - devq[0].pos;
            doneq->append(devq[0]);
            devq.removeFirst();
            // poke the mixer to update itself, note this is different
            // than windows, where a buffer is "done" after it is played
            // out (i think, at least that is what is implied in the ms docs.)
            // in this case, we signal it is "done" when it goes out to the
            // driver, but we don't know when it is actually going to be played
            // in real time (because it may just be q'd up in a buffer).
            //if(TheAudioMixer)
            //TheAudioMixer->done_callback();
            mixer_buf_done_callback();
        }
    }

    return len - toload;
}


void
mumble::qt_pushmore()
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return;
    if(!qio_dev || !audio_output)
        return;
    int len = audio_output->bytesFree();
    char *buf = new char[len];
    int len_out = qt_filler(0, buf, len);
    if(len_out > 0)
    {
        qio_dev->write(buf, len_out);
        GRTLOG("pushed %d", len, 0);
    }
    delete [] buf;
}

qint64
Audo_qio::readData(char *data, qint64 maxlen)
{
    QMutexLocker ml(&audio_mutex);
    qint64 ret = qt_filler(0, data, maxlen);
    GRTLOG("read data %ld", ret, 0);
    return ret;
}


void
DWYCOCALLCONV
audout_qt_new(void *)
{
    QMutexLocker ml(&audio_mutex);
    GRTLOG("qt new", 0, 0);
    if(Init)
        return;
    devq_p = new QList<audpack>;
    doneq = new QList<audpack>;
    // since this new is assumed to only be called from the
    // main thread, and not the mixer, this ensures m is
    // associated with the main thread and not the mixer
    m = new mumble;
    m->moveToThread(QGuiApplication::instance()->thread());

}

void
DWYCOCALLCONV
audout_qt_delete(void *)
{
    QMutexLocker ml(&audio_mutex);

    GRTLOG("qt del", 0, 0);
    delete m;
    delete devq_p;
    delete doneq;

    devq_p = 0;
    doneq = 0;
    m = 0;
    Init = 0;
}

int
DWYCOCALLCONV
audout_qt_init(void *)
{
    QMutexLocker ml(&audio_mutex);
    if(Init)
        return 1;
    GRTLOG("init", 0, 0);
    m->emit init();
    Init = 1;
    return 1;
}


int
DWYCOCALLCONV
audout_qt_device_output(void *, void *buf, int len, int user_data)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 0;

    int ret = 1;
    GRTLOG("qt dev out %d", len, 0);
    // q up the data so that it can be pushed out in the callback
    // function.
    struct audpack a;
    a.buf = (char *)buf;
    a.len = len;
    a.user_data = user_data;
    a.pos = 0;
    devq_p->append(a);
    GRTLOG("qt dev out state %d", m->audio_output->state(), 0);
    if(m->audio_output->state() != QAudio::ActiveState)
    {
        m->emit init();
    }
    return ret;
}

int
DWYCOCALLCONV
audout_qt_device_done(void *, void **buf_out, int *len, int *user_data)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 0;

    if(doneq->count() > 0)
    {
        GRTLOG("done %d", doneq->count(), 0);
        struct audpack a = doneq->at(0);
        *buf_out = a.buf;
        *len = a.len;
        *user_data = a.user_data;
        doneq->removeFirst();
        return 1;
    }
    return 0;

}

int
DWYCOCALLCONV
audout_qt_device_stop(void *)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 1;
    GRTLOG("stop", 0, 0);

    m->emit suspend();
    return 1;
}

int
DWYCOCALLCONV
audout_qt_device_reset(void *)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 1;
    GRTLOG("reset", 0, 0);
    m->emit reset();
    int n = devq_p->count();
    for(int i = 0; i < n; ++i)
    {
        doneq->append((*devq_p)[i]);
    }
    delete devq_p;
    devq_p = new QList<audpack>;

    return 1;
}

int
DWYCOCALLCONV
audout_qt_device_status(void *)
{
    QMutexLocker ml(&audio_mutex);
    // XXX fix me, may need some error indication
    return Init && m->audio_output && m->audio_output->error() == QAudio::NoError;
    return Init;
}

int
DWYCOCALLCONV
audout_qt_device_close(void *ah)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 1;
    GRTLOG("close", 0, 0);
    audout_qt_device_reset(ah);
    Init = 0;

    return 1;
}

int
DWYCOCALLCONV
audout_qt_device_buffer_time(void *, int sz)
{
    return 80;
}

int
DWYCOCALLCONV
audout_qt_device_play_silence(void *ah)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 1;
    char *sil = new char[AUDBUF_LEN];
    memset(sil, 0, AUDBUF_LEN);
    audout_qt_device_output(ah, sil, AUDBUF_LEN, 1);
    return 1;
}

int
DWYCOCALLCONV
audout_qt_device_bufs_playing(void *ah)
{
    QMutexLocker ml(&audio_mutex);
    if(!Init)
        return 0;
    int tmp = devq_p->count();
    GRTLOG("playing %d", tmp, 0);
    return tmp;
}
