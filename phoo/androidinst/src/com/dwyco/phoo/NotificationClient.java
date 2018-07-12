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

package com.dwyco.phoo;
//import com.dwyco.phoo.R;
import org.qtproject.qt5.android.bindings.QtActivity;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.os.Bundle;
import android.app.AlarmManager;
import java.util.Calendar;
import android.content.SharedPreferences;

import android.content.ContentResolver;

import android.database.Cursor;
import android.net.Uri;

import android.provider.ContactsContract;
import android.util.Log;
import android.os.Build;
import android.os.Build.VERSION;
import android.provider.DocumentsContract;
import android.content.ContentUris;
import android.os.Environment;
import android.provider.MediaStore;
import android.os.Vibrator;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import com.crashlytics.android.Crashlytics;

// note: use notificationcompat stuff for older androids

public class NotificationClient extends QtActivity
{
    private static NotificationClient m_instance;
    public static String msg_count_url;
    private static SocketLock prefs_lock;
    public static int allow_notification = 1;

    public NotificationClient()
    {
        m_instance = this;
        prefs_lock = new SocketLock("com.dwyco.phoo.prefs");
    }

    @Override
    public void onCreate(Bundle state) {
        super.onCreate(state);
        //Intent i = new Intent(m_instance, Push_Notification_Service.class);
        //startService(i);

        //Calendar cur_cal = Calendar.getInstance();
        //cur_cal.setTimeInMillis(System.currentTimeMillis());
        //Intent i2 = new Intent(m_instance, Push_Notification_Service.class);
        //PendingIntent pintent = PendingIntent.getService(m_instance, 0, i2, 0);
        //AlarmManager alarm = (AlarmManager) m_instance.getSystemService(Context.ALARM_SERVICE);
        //alarm.setRepeating(AlarmManager.RTC_WAKEUP, cur_cal.getTimeInMillis(), 1000 * 60, pintent);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
    }

    public static void set_allow_notification(int a)
    {
        allow_notification = a;
    }


    public static void notify(String s)
    {
        if(allow_notification == 0)
            return;
        NotificationManager m_notificationManager = (NotificationManager)m_instance.getSystemService(Context.NOTIFICATION_SERVICE);

        Notification.Builder m_builder = new Notification.Builder(m_instance);
        m_builder.setSmallIcon(R.drawable.ic_stat_not_icon2);
        m_builder.setColor(m_instance.getResources().getColor(R.color.green));
        m_builder.setContentTitle("Dwyco");
        m_builder.setAutoCancel(true);
        m_builder.setContentText("New messages");
        m_builder.setOnlyAlertOnce(true);
        SharedPreferences sp;
        prefs_lock.lock();
        sp = m_instance.getSharedPreferences("phoo", MODE_PRIVATE);
        int quiet = sp.getInt("quiet", 0);
        prefs_lock.release();
        int def = Notification.DEFAULT_ALL;
        if(quiet == 1)
            def = def & (~(Notification.DEFAULT_SOUND|Notification.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);

        Intent notintent = new Intent(m_instance, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(m_instance, 1, notintent, 0);
        m_builder.setContentIntent(p);

        Notification not = m_builder.getNotification();
        m_notificationManager.notify(1, not);

    }

    public static void cancel()
    {
        NotificationManager m_notificationManager = (NotificationManager)m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
        m_notificationManager.cancelAll();
    }

    public static void vibrate(long ms)
    {
        try {
            Vibrator v;
            v = (Vibrator)m_instance.getSystemService(Context.VIBRATOR_SERVICE);
            if(!v.hasVibrator())
                return;
            v.vibrate(ms);
        }
        catch(Exception e)
        {
            catchLog("vibrate exception " + e);
        }
    }

    public static void set_msg_count_url(String s) {
        prefs_lock.lock();
        msg_count_url = s;
        SharedPreferences sp = m_instance.getSharedPreferences("phoo", MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putString("url", s);
        pe.commit();
        prefs_lock.release();

    }

    public static void set_quiet(int i) {
        prefs_lock.lock();
        SharedPreferences sp = m_instance.getSharedPreferences("phoo", MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putInt("quiet", i);
        pe.commit();
        prefs_lock.release();
       // Crashlytics.getInstance().crash();
    }

    public static void set_service_params(int port, String sys_pfx, String user_pfx, String tmp_pfx) {
        prefs_lock.lock();
        SharedPreferences sp = m_instance.getSharedPreferences("phoo", MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putInt("lockport", port);
        pe.putString("sys_pfx", sys_pfx);
        pe.putString("user_pfx", user_pfx);
        pe.putString("tmp_pfx", tmp_pfx);
        pe.commit();
        prefs_lock.release();
        //FirebaseCrash.report(new Exception("My first Android non-fatal error"));

    }

    public static void start_background() {
        prefs_lock.lock();
        SharedPreferences sp;

        sp = m_instance.getSharedPreferences("phoo", MODE_PRIVATE);
        int port = sp.getInt("lockport", 4500);
        String sys_pfx = sp.getString("sys_pfx", ".");
        String user_pfx = sp.getString("user_pfx", ".");
        String tmp_pfx = sp.getString("tmp_pfx", ".");
        prefs_lock.release();
        // this is just a hack to avoid a crash in android O
        // this disables the background processing that happens when the
        // main app goes to sleep, which means delivery of large messages
        // will not work quite right (only happens when the app is
        // active). this will have to be fixed eventually.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            return;
            }

        Intent i = new Intent(m_instance, Dwyco_Message.class);
        i.putExtra("lockport", port);
        i.putExtra("sys_pfx", sys_pfx);
        i.putExtra("user_pfx", user_pfx);
        i.putExtra("tmp_pfx", tmp_pfx);

        m_instance.startService(i);

    }

    public static void load_contacts() {

        catchLog("load contacts");
        try {
            dwybg.dwyco_clear_contact_list();
            Uri uri = ContactsContract.CommonDataKinds.Email.CONTENT_URI;
            String selection = ContactsContract.Contacts.HAS_PHONE_NUMBER;
            Cursor cursor = m_instance.getContentResolver().query(uri, new String[] {
                //ContactsContract.CommonDataKinds.Phone.NUMBER,
                ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME,
                ContactsContract.CommonDataKinds.Email.ADDRESS,
                ContactsContract.CommonDataKinds.Phone._ID,
                ContactsContract.Contacts._ID
            },
            null,
            null,
            ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME + " ASC");

            cursor.moveToFirst();
            while (cursor.isAfterLast() == false) {
                String contactNumber;
                String contactName;
                String email;
                contactNumber = "";
                contactName = "";
                email = "";


                //contactNumber = cursor.getString(0);

                contactName = cursor.getString(0);

                email = cursor.getString(1);
                dwybg.dwyco_add_contact(contactName, contactNumber, email);


                //String contactNumber = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
                //String contactName = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME));
                //String email = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.ADDRESS));
                int phoneContactID = cursor.getInt(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone._ID));
                int contactID = cursor.getInt(cursor.getColumnIndex(ContactsContract.Contacts._ID));
                catchLog("name " + contactName + " num " + contactNumber + " email " + email);
                catchLog(" PhoneContactID " + phoneContactID + "  ContactID " + contactID);

                cursor.moveToNext();
            }
            cursor.close();
            cursor = null;
        }

        catch(Exception e)
        {
            catchLog("contact load failed" + e);
        }

    }

    // stuff related to image picking


    static final int REQUEST_OPEN_IMAGE = 1;


    public static void openAnImage()
    {
        m_instance.dispatchOpenGallery();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        catchLog("result");
        if (resultCode == RESULT_OK)
        {
            if(requestCode == REQUEST_OPEN_IMAGE)
            {
                String filePath = getPath(getApplicationContext(), data.getData());
                if(filePath != null)
                {
                    dwybg.dwyco_set_aux_string(filePath);
                    catchLog("result " + filePath);
                }
                else
                {
                    dwybg.dwyco_set_aux_string("");
                    catchLog("result null");
                }

            }
        }
        else
        {
            dwybg.dwyco_set_aux_string("");
            catchLog("result nope");
        }

        super.onActivityResult(requestCode, resultCode, data);
    }

    private void dispatchOpenGallery()
    {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("image/*");
        startActivityForResult(intent, REQUEST_OPEN_IMAGE);
    }

    /**
     * Get a file path from a Uri. This will get the the path for Storage Access
     * Framework Documents, as well as the _data field for the MediaStore and
     * other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri The Uri to query.
     * @author paulburke
     */
    public static String getPath(final Context context, final Uri uri) {

        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;

        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }

                // TODO handle non-primary volumes
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {

                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                                           Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));

                return getDataColumn(context, contentUri, null, null);
            }
            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                final String selection = "_id=?";
                final String[] selectionArgs = new String[] {
                    split[1]
                };

                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        // MediaStore (and general)
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            return getDataColumn(context, uri, null, null);
        }
        // File
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }

        return null;
    }

    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri The Uri to query.
     * @param selection (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    public static String getDataColumn(Context context, Uri uri, String selection,
                                       String[] selectionArgs) {

        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {
            column
        };

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                     null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndex(column);
                if(column_index == -1)
                    return null;
                return cursor.getString(column_index);
            }
        }
        finally {
            if (cursor != null)
                cursor.close();
        }
        return null;
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    private static void catchLog(String log) {
        Log.d("dwyco_notfication", log);
    }

}


