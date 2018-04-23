
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AUDI_QT_H
#define AUDI_QT_H
#include <QObject>
#include <QIODevice>
#include <QAudioInput>
#include "dlli.h"

class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const { return m_level; }

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
    QAudioDeviceInfo m_device;
    AudioInfo *m_audioInfo;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
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

void DWYCOCALLCONV audi_qt_new(void *, int buflen, int nbufs);
void DWYCOCALLCONV audi_qt_delete(void *);
int DWYCOCALLCONV audi_qt_init(void *);
int DWYCOCALLCONV audi_qt_has_data(void *);
void DWYCOCALLCONV audi_qt_need(void *);
void DWYCOCALLCONV audi_qt_pass(void *);
void DWYCOCALLCONV audi_qt_stop(void *);
void DWYCOCALLCONV audi_qt_on(void *);
void DWYCOCALLCONV audi_qt_off(void *);
void DWYCOCALLCONV audi_qt_reset(void *);
int DWYCOCALLCONV audi_qt_status(void *);
void * DWYCOCALLCONV audi_qt_get_data(void *, int *r, int *c);


#endif // AUDI_QT_H
