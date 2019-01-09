package com.dwyco.phoo;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.util.Log;
import android.app.Service;
import android.os.Binder;
import android.os.IBinder;
import android.content.SharedPreferences;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.File;
import java.util.Arrays;
import android.os.Build;
import android.os.Build.VERSION;
import android.app.job.JobParameters;
import android.app.job.JobService;


public class DwycoProbe extends JobService {

    private static Context context;
    private static String[] local_files = {};
    private static SocketLock prefs_lock;

    public DwycoProbe() {
        //super("DwycoProbe");
        prefs_lock = new SocketLock("com.dwyco.phoo.prefs");

    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        catchLog("DwycoProbe Service got created");
        context = this;
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("dwyco_jni");
    }


    @Override
    public boolean onStartJob(final JobParameters params) {
        //JobWorkItem jwi = params.dequeueWork();
        //if(jwi == null)
        //    return false;

        Thread t = new Thread(new Runnable() {
            public void run() {
        catchLog("DwycoProbe starting");
        prefs_lock.lock();
        SharedPreferences sp;
        int port;
        String sys_pfx;
        String user_pfx;
        String tmp_pfx;
        String token;

            sp = context.getSharedPreferences("dwyco_msg", MODE_PRIVATE);
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
        //poller_thread();
        
        //set_notification();
        dwybg.dwyco_background_processing(port, 0, sys_pfx, user_pfx, tmp_pfx, token);
        catchLog("job end");
        jobFinished(params, true);
        System.exit(0);
            }
        }
        );
        t.start();
        return true;
        //System.exit(0);

    }
    @Override
    public boolean onStopJob(JobParameters params) {
        catchLog("STOP JOB");
        System.exit(0);
        return true;
    }

    

    private void set_notification() {
        NotificationManager m_notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);

        Notification.Builder m_builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        m_builder = new Notification.Builder(context, "dwyco");
        } else {
        m_builder = new Notification.Builder(context);
        }
        m_builder.setSmallIcon(R.drawable.ic_stat_not_icon2);
        //m_builder.setColor(context.getResources().getColor(R.color.green));
        m_builder.setContentTitle("Dwyco");
        m_builder.setAutoCancel(true);
        m_builder.setContentText("Message probing");
        m_builder.setOnlyAlertOnce(true);
        SharedPreferences sp;
        prefs_lock.lock();
        sp = context.getSharedPreferences("phoo", MODE_PRIVATE);
        int quiet = sp.getInt("quiet", 0);
        prefs_lock.release();
        int def = Notification.DEFAULT_ALL;
        if(quiet == 1)
            def = def & (~(Notification.DEFAULT_SOUND|Notification.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);

        Intent notintent = new Intent(context, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, 0);
        m_builder.setContentIntent(p);

        Notification not = m_builder.getNotification();
        //m_notificationManager.notify(1, not);
        startForeground(1, not);
    }

    private void clear_notification() {
        NotificationManager m_notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        m_notificationManager.cancelAll();
    }

    private void catchLog(String log) {
        Log.d("DwycoProbe", log);


    }

}
