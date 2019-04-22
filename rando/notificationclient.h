/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAndroidExtras module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef NOTIFICATIONCLIENT_H
#define NOTIFICATIONCLIENT_H
#ifdef ANDROID
#include <QObject>

class NotificationClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString notification READ notification WRITE setNotification NOTIFY notificationChanged)
public:
    explicit NotificationClient(QObject *parent = 0);

    void setNotification(const QString &notification);
    QString notification() const;
    void set_allow_notification(int);
    QString get_token();


signals:
    void notificationChanged();

public slots:
    void updateAndroidNotification();
    void cancel();
    void set_msg_count_url(QString s);
    void set_quiet(int i);
    void set_service_params(int port, QString sys_pfx, QString user_pfx, QString tmp_pfx);
    void start_background();
    void load_contacts();
    void open_image();
    void vibrate(long ms);
    void log_event();
    void set_user_property(QString name, QString value);
    void set_lastrun();

private:
    QString m_notification;
    int allow_notification;
};
extern NotificationClient *notificationClient;
#endif

#endif // NOTIFICATIONCLIENT_H
