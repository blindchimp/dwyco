package com.dwyco.android;
import com.dwyco.cdc32.dwybg;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.util.Log;
import android.content.SharedPreferences;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.File;
import java.util.Arrays;
import java.net.*;
import android.os.Build;
import android.os.Build.VERSION;
import androidx.work.Worker;
import androidx.work.ForegroundInfo;
import androidx.work.WorkerParameters;

public class DwycoProbe extends Worker {

    private Context context;
    private SocketLock prefs_lock;
    private boolean stop_poller;

    public DwycoProbe(
        Context context,
        WorkerParameters params) {
        super(context, params);
        this.context = context;
        prefs_lock = new SocketLock(DwycoApp.lock_shared_prefs);
        stop_poller = false;
        // note, docs seem to indicate loading a library
        // multiple times results in the second and further
        // loads being ignored. i hope so.
        System.loadLibrary("c++_shared");
        System.loadLibrary("dwyco_jni");
    }

    public ForegroundInfo getForegroundInfo() {
        Notification.Builder m_builder;
        m_builder = new Notification.Builder(context, "dwyco");
        m_builder.setContentTitle("Dwyco");
        m_builder.setAutoCancel(true);
        m_builder.setContentText("Uploading");
        m_builder.setOnlyAlertOnce(true);
        Notification not = m_builder.getNotification();
        ForegroundInfo f = new ForegroundInfo(1, not);
        return f;
    }

    public void onStopped() {
        
        prefs_lock.lock();
        SharedPreferences sp;
        int port;
        sp = context.getSharedPreferences(DwycoApp.shared_prefs, Context.MODE_PRIVATE);
        port = sp.getInt("lockport", 4500);
        prefs_lock.release();
        stop_poller = true;
        try
        {
            Socket s = new Socket(Inet4Address.getLoopbackAddress(), port);       
        }
        catch(Exception e)
        {
            catchLog("work stopped (already dead)");
            catchLog(e.getMessage());
        }
        finally
        {
            catchLog("work stopped");
        }
        
    }

    @Override
    public Result doWork() {
        catchLog("DwycoProbe starting");
        prefs_lock.lock();
        SharedPreferences sp;
        int port;
        String sys_pfx;
        String user_pfx;
        String tmp_pfx;
        String token;

        sp = context.getSharedPreferences(DwycoApp.shared_prefs, Context.MODE_PRIVATE);
        port = sp.getInt("lockport", 4500);
        sys_pfx = sp.getString("sys_pfx", ".");
        user_pfx = sp.getString("user_pfx", ".");
        tmp_pfx = sp.getString("tmp_pfx", ".");
        token = sp.getString("token", "notoken");
        
        prefs_lock.release();
        catchLog(sys_pfx);
        catchLog(user_pfx);
        catchLog(tmp_pfx);
        catchLog(String.valueOf(port));
        catchLog(token);
        // note: this is just in case we get a received message while
        // we are doing the sends. might want to consider just inhibiting
        // anything in the way of receive processing, but i don't think
        // this hurts anything.
        poller_thread();
        
        dwybg.dwyco_background_processing(port, 0/*DwycoApp.exit_if_outq_empty|DwycoApp.check_backup_once*/, sys_pfx, user_pfx, tmp_pfx, token);
        stop_poller = true;
        dwybg.dwyco_signal_msg_cond();
        catchLog("job end");
        return Result.success();
    }

    private void poller_thread() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                catchLog("poll thread");
                while(!stop_poller)
                {
                    dwybg.dwyco_wait_msg_cond(0);
                    if(stop_poller) {
                        break;
                    }
                    set_notification();
                }
                catchLog("poll done");
            }
        });
        t.start();
    }

    private void set_notification() {
        NotificationManager m_notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        
        SharedPreferences sp;
        prefs_lock.lock();
        sp = context.getSharedPreferences(DwycoApp.shared_prefs, Context.MODE_PRIVATE);
        int quiet = sp.getInt("quiet", 0);
        prefs_lock.release();
        Notification.Builder m_builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            if(quiet == 0) 
                m_builder = new Notification.Builder(context, "dwyco");
            else
                m_builder = new Notification.Builder(context, "dwyco-quiet");

        } else {
        m_builder = new Notification.Builder(context);
        int def = Notification.DEFAULT_ALL;
        if(quiet == 1)
            def = def & (~(Notification.DEFAULT_SOUND|Notification.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);
        }
        //m_builder.setSmallIcon(R.drawable.ic_stat_not_icon2);
        m_builder.setSmallIcon(DwycoApp.notification_icon());
        //m_builder.setColor(context.getResources().getColor(R.color.green));
        m_builder.setContentTitle("Dwyco");
        m_builder.setAutoCancel(true);
        m_builder.setContentText("Message received");
        m_builder.setOnlyAlertOnce(true);
        
        Intent notintent = new Intent(context, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, PendingIntent.FLAG_IMMUTABLE);
        m_builder.setContentIntent(p);

        Notification not = m_builder.getNotification();
        m_notificationManager.notify(1, not);
    }

    private void clear_notification() {
        NotificationManager m_notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        m_notificationManager.cancelAll();
    }

    private void catchLog(String log) {
        Log.d("DwycoProbe", log);


    }

}
