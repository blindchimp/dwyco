
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "audi_qt.h"

#include <QVector>
#include <QByteArray>
#include <QAudioInput>
#include <QObject>
#include <QQueue>
#include <QMutexLocker>
#include <QIODevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSource>

#ifdef ANDROID
//#include <QtAndroid>
#endif

void oopanic(const char *);

struct InputTest;

static QQueue<unsigned char *> Bufs;
static QQueue<qint64> Buf_times;
static int Buflen;
static int Cur_idx;
static int Max_queue_len;
static unsigned char *Cur_buf;
static int Cur_time;
static InputTest *Audi;
static QMutex Audi_mutex;

#define BufferSize 4096
// ack, 44.1khz * 80ms * 2 bytes
//#define AUDBUF_LEN (7056)
#if defined(ANDROID) || defined(DWYCO_IOS)
#define UWB_SAMPLE_RATE 8000
#else
#define UWB_SAMPLE_RATE 44100
#endif
#define AUDBUF_LEN ((int)(UWB_SAMPLE_RATE * .080 * 2))
class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const {
        return m_level;
    }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude;
    qreal m_level; // 0.0 <= m_level <= 1.0

signals:
    void update();
};

struct InputTest : public QObject
{
    Q_OBJECT

public:
    InputTest();
    ~InputTest();

    int initializeAudio();
    void createAudioInput();
    QAudioDevice m_device;
    AudioInfo *m_audioInfo;
    QAudioFormat m_format;
    QAudioSource *m_audioInput;
    QIODevice *m_input;
    bool m_pullMode;
    QByteArray m_buffer;

public slots:
    void readMore();
    void toggleMode();
    void toggleSuspend();

    void do_on();
    void do_off();

signals:
    void on();
    void off();

};

#include "audi_qt6.moc"

InputTest::InputTest() :
    m_device(QMediaDevices::defaultAudioInput())
    ,   m_audioInfo(0)
    ,   m_audioInput(0)
    ,   m_input(0)
    ,   m_pullMode(true)
    ,   m_buffer(BufferSize, 0)
{
}

InputTest::~InputTest()
{
    delete m_audioInfo;
	if(!m_audioInput)
		return;
    m_audioInput->reset();
    m_audioInput->deleteLater();
    m_audioInput = 0;
}

int InputTest::initializeAudio()
{
    m_format.setSampleRate(UWB_SAMPLE_RATE);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);
    m_format.setChannelConfig(QAudioFormat::ChannelConfigMono);


    if (!m_device.isFormatSupported(m_format)) {
        //qWarning() << "Default format not supported - trying to use nearest";
        //m_format = info.nearestFormat(m_format);
        return 0;
    }

    if (m_audioInfo)
        delete m_audioInfo;
    m_audioInfo  = new AudioInfo(m_format, this);
    //connect(m_audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));

    createAudioInput();
    if(m_audioInput->error() != QAudio::NoError)
        return 0;
    return 1;
}

void InputTest::createAudioInput()
{
    m_audioInput = new QAudioSource(m_device, m_format, this);
    //qreal initialVolume = QAudio::convertVolume(m_audioInput->volume(),
    //                                            QAudio::LinearVolumeScale,
    //                                            QAudio::LogarithmicVolumeScale);
    //m_volumeSlider->setValue(qRound(initialVolume * 100));
    m_audioInfo->start();
    m_audioInput->start(m_audioInfo);
    connect(this, &InputTest::on, this, &InputTest::do_on, Qt::QueuedConnection);
    connect(this, &InputTest::off, this, &InputTest::do_off, Qt::QueuedConnection);
}

void InputTest::readMore()
{
    if (!m_audioInput)
        return;
    qint64 len = m_audioInput->bytesAvailable();
    if (len > BufferSize)
        len = BufferSize;
    qint64 l = m_input->read(m_buffer.data(), len);
    if (l > 0)
        m_audioInfo->write(m_buffer.constData(), l);
}

void InputTest::toggleMode()
{
    // Change bewteen pull and push modes
    m_audioInput->stop();

    if (m_pullMode) {
        //m_modeButton->setText(tr(PULL_MODE_LABEL));
        m_input = m_audioInput->start();
        connect(m_input, SIGNAL(readyRead()), SLOT(readMore()));
        m_pullMode = false;
    } else {
        //m_modeButton->setText(tr(PUSH_MODE_LABEL));
        m_pullMode = true;
        m_audioInput->start(m_audioInfo);
    }

    // m_suspendResumeButton->setText(tr(SUSPEND_LABEL));
}

void InputTest::toggleSuspend()
{
    // toggle suspend/resume
    if (m_audioInput->state() == QAudio::SuspendedState) {
        m_audioInput->resume();
        //m_suspendResumeButton->setText(tr(SUSPEND_LABEL));
    } else if (m_audioInput->state() == QAudio::ActiveState) {
        m_audioInput->suspend();
        //m_suspendResumeButton->setText(tr(RESUME_LABEL));
    } else if (m_audioInput->state() == QAudio::StoppedState) {
        m_audioInput->resume();
        //m_suspendResumeButton->setText(tr(SUSPEND_LABEL));
    } else if (m_audioInput->state() == QAudio::IdleState) {
        // no-op
    }
}

void
InputTest::do_on()
{
    if(!m_audioInput)
        return;
    if(m_audioInput->state() != QAudio::SuspendedState)
        return;
    m_audioInput->resume();

}

void
InputTest::do_off()
{
    if(!m_audioInput)
        return;
    if(m_audioInput->state() != QAudio::ActiveState)
        return;
    m_audioInput->suspend();
}

AudioInfo::AudioInfo(const QAudioFormat &format, QObject *parent)
    :   QIODevice(parent)
    ,   m_format(format)
    ,   m_maxAmplitude(0)
    ,   m_level(0.0)

{

}

AudioInfo::~AudioInfo()
{
}

void AudioInfo::start()
{
    open(QIODevice::WriteOnly);
}

void AudioInfo::stop()
{
    close();
}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)

    return 0;
}

qint64 AudioInfo::writeData(const char *data, qint64 len)
{
    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    int total_len = len;
    do
    {
        int to_cpy = Buflen - Cur_idx;
        to_cpy = total_len < to_cpy ? total_len : to_cpy;

        memcpy(Cur_buf + Cur_idx, ptr, to_cpy);
        total_len -= to_cpy;
        Cur_idx += to_cpy;
        ptr += to_cpy;
        if(Cur_idx == Buflen)
        {
            Bufs.enqueue(Cur_buf);
            //Buf_times.enqueue(Audi->m_audioInput->elapsedUSecs() - 80000);
            Cur_buf = new unsigned char[Buflen];
            Cur_idx = 0;
        }
    }
    while(total_len > 0);

    return len;
}


void DWYCOCALLCONV
audi_qt_new(void *, int buflen, int nbufs)
{
    Max_queue_len = nbufs;
    Buflen = buflen;
    Cur_idx = 0;
    Cur_buf = new unsigned char[buflen];
    Cur_time = 0;

}
void DWYCOCALLCONV
audi_qt_delete(void *)
{
    for(int i = 0; i < Bufs.count(); ++i)
    {
        delete [] Bufs[i];
    }
    Bufs.clear();
    Buf_times.clear();
    //delete Audi;
    Audi->deleteLater();
    Audi = 0;
}

#ifdef ANDROID
static
void
nada(const QtAndroid::PermissionResultMap&)
{
    return;
}
#endif

int DWYCOCALLCONV
audi_qt_init(void *)
{
#ifdef ANDROID
    if(QtAndroid::checkPermission("android.permission.RECORD_AUDIO") == QtAndroid::PermissionResult::Denied)
    {
        QtAndroid::PermissionResultMap m = QtAndroid::requestPermissionsSync(QStringList("android.permission.RECORD_AUDIO"));
        if(m.value("android.permission.RECORD_AUDIO") == QtAndroid::PermissionResult::Denied)
        {
            return 0;
        }
    }
#endif
    if(Audi)
        return 1;
    Audi = new InputTest;
    if(Audi->initializeAudio() == 0)
    {
        delete Audi;
        Audi = 0;
        return 0;
    }
    return 1;
}

int DWYCOCALLCONV
audi_qt_has_data(void *)
{
    if(Bufs.count() > 0)
        return 1;
    return 0;
}

void DWYCOCALLCONV
audi_qt_need(void *)
{
    audi_qt_init(0);

}
void DWYCOCALLCONV audi_qt_pass(void *)
{
    oopanic("audi_pass");
}
void DWYCOCALLCONV audi_qt_stop(void *)
{
    oopanic("audi_stop");
}
void DWYCOCALLCONV audi_qt_on(void *)
{
    QMutexLocker m(&Audi_mutex);
    if(!Audi)
        return;
    Audi->emit on();
#if 0
    if(Audi->m_audioInput->state() != QAudio::SuspendedState)
        return;
    Audi->m_audioInput->resume();
#endif

}
void DWYCOCALLCONV audi_qt_off(void *)
{
    QMutexLocker m(&Audi_mutex);
    if(!Audi)
        return;
    Audi->emit off();
#if 0
    if(Audi->m_audioInput->state() != QAudio::ActiveState)
        return;
    Audi->m_audioInput->suspend();
#endif

}
void DWYCOCALLCONV audi_qt_reset(void *)
{
    if(!Audi)
        return;
    if(Audi->m_audioInput)
        Audi->m_audioInput->stop();
//    for(int i = 0; i < Bufs.count(); ++i)
//    {
//        delete [] Bufs[i];
//    }
//    Bufs.clear();
//    Buf_times.clear();
    audi_qt_delete(0);

}
int DWYCOCALLCONV audi_qt_status(void *)
{
    if(Audi && Audi->m_audioInput && Audi->m_audioInput->state() == QAudio::ActiveState)
        return 1;
    return 0;

}
void * DWYCOCALLCONV audi_qt_get_data(void *, int *r, int *c)
{
    if(Bufs.count() == 0)
        oopanic("bad audio get");

    unsigned char *buf = Bufs.dequeue();
    //qint64 tm = Buf_times.dequeue();

    *r = Buflen;
    *c = Cur_time;
    Cur_time += Buflen;
    return buf;

}
