
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef callsm_objs_h
#define callsm_objs_h


#include <QObject>
#include <QQmlObjectListModel.h>
#include <QColor>

class FauxButton : public QObject
{
    Q_OBJECT
public:
    FauxButton(QObject *parent = 0) : QObject(parent) {
        m_checked = 0;
        m_enabled = 1;
        m_visible = 1;
        m_checkable = 0;
        m_BackgroundColor = QColor("green");
    }

    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable NOTIFY checkableChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    bool checked() const {
        return m_checked;
    }
    bool checkable() const {
        return m_checkable;
    }
    bool visible() const {
        return m_visible;
    }
    bool enabled() const {
        return m_enabled;
    }
    QString text() const {
        return m_text;
    }
    QColor backgroundColor() const {
        return m_BackgroundColor;
    }

public slots:
    void trigger() {
        if(!m_enabled)
            return;
        if(m_checkable)
        {
            setChecked(!m_checked);
        }
        if(m_checkable)
            emit triggered(m_checked);
        else
            emit triggered();

    }

    void toggle() {
        if(!m_enabled)
            return;
        if(m_checkable)
        {
            setChecked(!m_checked);
        }
        if(m_checkable)
            emit toggled(m_checked);
    }

    void setChecked(bool a) {
        if(m_checked != a)
        {
            m_checked = a;
            emit checkedChanged(a);
            emit toggled(a);
        }
    }

    void setCheckable(bool a) {
        if(m_checkable != a)
        {
            m_checkable = a;
            emit checkableChanged(a);

        }
    }

    void setVisible(bool a) {
        if(m_visible != a)
        {
            m_visible = a;
            emit visibleChanged(a);
        }
    }

    void setEnabled(bool a) {
        if(m_enabled != a)
        {
            m_enabled = a;
            emit enabledChanged(a);
        }
    }

    void setText(QString a) {
        if(m_text != a)
        {
            m_text = a;
            emit textChanged(a);
        }
    }

    int isChecked() {
        return m_checked;
    }

    void click() {
        if(!m_enabled)
            return;
        if(m_checkable)
        {
            setChecked(!m_checked);
        }
        if(m_checkable)
            emit clicked(m_checked);
        else
            emit clicked();
    }

    void setBackgroundColor(QColor c) {
        if(m_BackgroundColor != c)
        {
            m_BackgroundColor = c;
            emit backgroundColorChanged(c);
        }

    }


signals:
    void clicked(bool checked);
    void clicked();
    void checkedChanged(bool);
    void visibleChanged(bool);
    void enabledChanged(bool);
    void textChanged(QString);
    void checkableChanged(bool);
    void triggered(bool a = false);
    void toggled(bool);
    void pressed();
    void released();
    void backgroundColorChanged(QColor);

private:
    bool m_checked;
    bool m_visible;
    bool m_enabled;
    bool m_checkable;
    QString m_text;
    QColor m_BackgroundColor;

};

class callsm_objs
{
public:
    QQmlObjectListModel<FauxButton> *button_model;

    FauxButton *actionPause;
    FauxButton *actionIgnore_user_ctx;
    FauxButton *actionCam_off;
    FauxButton *actionCam_on;
    FauxButton *accept;
    FauxButton *accept_and_send;
    FauxButton *send_video;
    FauxButton *cancel_req;
    FauxButton *send_audio_button;
    FauxButton *mute_button;
    FauxButton *hangup;
    FauxButton *reject;

    void setupUi(QObject *simple_call)
    {
        if (simple_call->objectName().isEmpty())
            simple_call->setObjectName(QString::fromUtf8("simple_call"));
        actionPause = new FauxButton(simple_call);
        actionPause->setObjectName(QString::fromUtf8("actionPause"));
        actionPause->setCheckable(true);
        actionPause->setText("Pause");
        actionIgnore_user_ctx = new FauxButton(simple_call);
        actionIgnore_user_ctx->setObjectName(QString::fromUtf8("actionIgnore_user_ctx"));
        actionIgnore_user_ctx->setText("Block");
        actionCam_off = new FauxButton(simple_call);
        actionCam_off->setObjectName(QString::fromUtf8("actionCam_off"));
        actionCam_off->setCheckable(true);
        actionCam_off->setText("Cam off");
        actionCam_off->setVisible(false);
        actionCam_on = new FauxButton(simple_call);
        actionCam_on->setObjectName(QString::fromUtf8("actionCam_on"));
        actionCam_on->setCheckable(true);
        actionCam_on->setText("Cam on");
        actionCam_on->setVisible(false);

        accept = new FauxButton(simple_call);
        accept->setObjectName(QString::fromUtf8("accept"));
        accept->setText("Accept");

        accept_and_send = new FauxButton(simple_call);
        accept_and_send->setObjectName(QString::fromUtf8("accept_and_send"));
        accept_and_send->setText("Accept and send");

        send_video = new FauxButton(simple_call);
        send_video->setObjectName(QString::fromUtf8("send_video"));
        send_video->setText("Send video");

        cancel_req = new FauxButton(simple_call);
        cancel_req->setObjectName(QString::fromUtf8("cancel_req"));
        cancel_req->setText("Cancel");

        send_audio_button = new FauxButton(simple_call);
        send_audio_button->setObjectName(QString::fromUtf8("send_audio_button"));
        send_audio_button->setCheckable(true);
        send_audio_button->setText("Send audio");

        mute_button = new FauxButton(simple_call);
        mute_button->setObjectName(QString::fromUtf8("mute_button"));
        mute_button->setCheckable(true);
        mute_button->setText("Mute");

        hangup = new FauxButton(simple_call);
        hangup->setObjectName(QString::fromUtf8("hangup"));
        hangup->setText("Hang up");

        reject = new FauxButton(simple_call);
        reject->setObjectName(QString::fromUtf8("reject"));
        reject->setText("Reject");

        QMetaObject::connectSlotsByName(simple_call);

        button_model = new QQmlObjectListModel<FauxButton>(simple_call, "text", "objectName");
        const QObjectList& cl = simple_call->children();
        for(int i = 0; i < cl.count(); ++i)
        {
            cl[i]->setProperty("visible", false);
            button_model->append(cl[i]);
        }
    }


};


#endif
