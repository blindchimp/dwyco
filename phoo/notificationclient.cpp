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
#ifdef ANDROID
#include "notificationclient.h"

#include <QJniObject>
//#include <QtAndroidExtras>
typedef QJniObject QAndroidJniObject;

void android_log_stuff(const char *str, const char *s1, int s2);


NotificationClient::NotificationClient(QObject *parent)
    : QObject(parent)
{
    connect(this, SIGNAL(notificationChanged()), this, SLOT(updateAndroidNotification()));
}

void NotificationClient::setNotification(const QString &notification)
{
    if (m_notification == notification)
        return;

    m_notification = notification;
    emit notificationChanged();
}

QString NotificationClient::notification() const
{
    return m_notification;
}

void NotificationClient::updateAndroidNotification()
{
    if(m_notification.length() == 0)
        return;
    QAndroidJniObject javaNotification = QAndroidJniObject::fromString(m_notification);
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "notify",
            "(Ljava/lang/String;)V",
            javaNotification.object<jstring>());
}

void NotificationClient::postAndroidNotification(const QString& notification)
{

    QAndroidJniObject javaNotification = QAndroidJniObject::fromString(notification);
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "notify",
            "(Ljava/lang/String;)V",
            javaNotification.object<jstring>());
}

void
NotificationClient::set_allow_notification(int a)
{
    jint jms = a;
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "set_allow_notification",
            "(I)V",
            jms);

}

void NotificationClient::cancel()
{
    m_notification = "";
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "cancel");
}

void NotificationClient::set_lastrun()
{
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "set_lastrun");
}

void NotificationClient::takePicture()
{
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "openCamera");
}

void
NotificationClient::set_msg_count_url(QString s)
{
    QAndroidJniObject jurl = QAndroidJniObject::fromString(s);
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "set_msg_count_url",
            "(Ljava/lang/String;)V",
            jurl.object<jstring>());

}

void
NotificationClient::set_service_params(int port, QString sys_pfx, QString user_pfx, QString tmp_pfx)
{
    QAndroidJniObject jsys = QAndroidJniObject::fromString(sys_pfx);
    QAndroidJniObject juser = QAndroidJniObject::fromString(user_pfx);
    QAndroidJniObject jtmp = QAndroidJniObject::fromString(tmp_pfx);
    jint jport = port;
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "set_service_params",
            "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            jport,
            jsys.object<jstring>(),
            juser.object<jstring>(),
            jtmp.object<jstring>()
                                             );

}

void
NotificationClient::start_background()
{
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "start_background"
    );

}

void
NotificationClient::log_event()
{
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "log_event"
    );

}

void
NotificationClient::log_event2(QString name, QString method)
{
    QAndroidJniObject jname = QAndroidJniObject::fromString(name);
    QAndroidJniObject jval = QAndroidJniObject::fromString(method);
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "log_event2",
      "(Ljava/lang/String;Ljava/lang/String;)V",
      jname.object<jstring>(),
      jval.object<jstring>()
    );

}

void
NotificationClient::set_user_property(QString name, QString value)
{
    QAndroidJniObject jname = QAndroidJniObject::fromString(name);
    QAndroidJniObject jval = QAndroidJniObject::fromString(value);

    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
                                              "set_user_property",
                                              "(Ljava/lang/String;Ljava/lang/String;)V",

                                              jname.object<jstring>(),
                                              jval.object<jstring>()
                                              );
}

void
NotificationClient::share_to_mediastore(QString filename)
{
    QAndroidJniObject jname = QAndroidJniObject::fromString(filename);


    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
                                              "notifyMediaStoreScanner",
                                              "(Ljava/lang/String;)V",

                                              jname.object<jstring>()
                                              );
}

void
NotificationClient::load_contacts()
{
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "load_contacts"
    );

}

int
NotificationClient::open_image()
{
#if 0
    QtAndroid::PermissionResultMap m;

#if 0
    if(QtAndroid::checkPermission("android.permission.READ_MEDIA_IMAGES") == QtAndroid::PermissionResult::Granted)
        goto ok;


    m = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_MEDIA_IMAGES"));
    if(m.value("android.permission.READ_MEDIA_IMAGES") == QtAndroid::PermissionResult::Granted)
        goto ok;
#endif

    if(QtAndroid::androidSdkVersion() < 30)
    {
        if(QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Granted)
            goto ok;

        m = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_EXTERNAL_STORAGE"));
        if(m.value("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
        {
            return 0;
        }
    }
#endif

ok:;
    QAndroidJniObject::callStaticMethod<void>(
        "com/dwyco/android/NotificationClient",
        "openAnImage"
    );
    return 1;
}

void
NotificationClient::vibrate(long ms)
{
    jlong jms = ms;
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "vibrate",
            "(J)V",
            jms);

}

void
NotificationClient::beep()
{
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
                                              "beep"
                                              );

}

void
NotificationClient::set_quiet(int i)
{
    jint ji = i;
    QAndroidJniObject::callStaticMethod<void>("com/dwyco/android/NotificationClient",
            "set_quiet",
            "(I)V",
            ji);

}

QString
NotificationClient::get_token()
{
    QAndroidJniObject token = QAndroidJniObject::callStaticObjectMethod<jstring>(
                "com/dwyco/android/NotificationClient",
                "get_token"
                );

    return token.toString();
}

#endif
