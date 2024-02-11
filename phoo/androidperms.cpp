
#include "androidperms.h"
#ifdef ANDROID
#include <QPermissions>
#include <QCoreApplication>
#endif

AndroidPerms::AndroidPerms(QObject *parent) : QObject(parent)
{
    m_camera_permission = false;
    m_external_storage_permission = false;
    m_post_notifications_permission = false;

}

int
AndroidPerms::android_api()
{
#ifndef ANDROID
    return 0;
#else
    return QNativeInterface::QAndroidApplication::sdkVersion();
#endif
}

void
AndroidPerms::load()
{
#ifndef ANDROID
    update_external_storage_permission(true);
    update_camera_permission(true);
    update_post_notifications_permission(true);
#else
    if(android_api() < 23)
    {
        update_external_storage_permission(true);
        update_camera_permission(true);
        update_post_notifications_permission(true);
    }
    else
    {
        if(QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Granted)
        {
            update_external_storage_permission(true);
        }
        else
        {
            update_external_storage_permission(false);
        }

        if(QtAndroid::checkPermission("android.permission.CAMERA") == QtAndroid::PermissionResult::Granted)
        {
            update_camera_permission(true);
        }
        else
        {
            update_camera_permission(false);
        }
        if(QtAndroid::checkPermission("android.permission.POST_NOTIFICATIONS") == QtAndroid::PermissionResult::Granted)
        {
            update_post_notifications_permission(true);
        }
        else
        {
            update_post_notifications_permission(false);
        }
    }
#endif

}

void
AndroidPerms::toggle()
{
    update_camera_permission(!m_camera_permission);
    update_external_storage_permission(!m_external_storage_permission);
}

bool
AndroidPerms::request_sync(QString perm)
{
#ifndef ANDROID
    return true;
#else
    QtAndroid::PermissionResultMap m = QtAndroid::requestPermissionsSync(QStringList(perm));
    if(m.value(perm) == QtAndroid::PermissionResult::Denied)
    {
        // this needs to be thought out a little more... if you deny this, you can't
        // access your photos on the device easily. maybe need to just request "read"
        // in this case.
        load();
        return false;
    }
    load();
    return true;
#endif
}
