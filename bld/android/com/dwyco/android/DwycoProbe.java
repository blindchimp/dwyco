package com.dwyco.android;
import com.dwyco.cdc32.dwybg;
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
        prefs_lock = new SocketLock(DwycoApp.lock_shared_prefs);

    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        catchLog("DwycoProbe Service got created");
        context = this;
        System.loadLibrary("c++_shared");
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

            sp = context.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
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
        poller_thread();
        
        //set_notification();
        dwybg.dwyco_background_processing(port, 1, sys_pfx, user_pfx, tmp_pfx, token);
        catchLog("job end");
        // release wakelock, we don't really need it after sending
        // whatever is in the q
        jobFinished(params, true);
        // ok, this is a little sneaky... it appears that the jobscheduler does not
        // immediately destroy the process, keeping it around for a few minutes
        // after there are no wakelocks (killing the process because it is "empty").
        // this is ok for us, because a few minutes after a send is probably the most
        // likely time we will get a reply, and this will expedite it in most cases
        // (since FCM takes 15 or 20 minutes sometimes to deliver a "high" priority
        // message). 
        dwybg.dwyco_background_processing(port, 0, sys_pfx, user_pfx, tmp_pfx, token);
        catchLog("job end2");
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

    private void poller_thread() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                catchLog("poll thread");
                while(true)
                {
                    dwybg.dwyco_wait_msg_cond(0);
                    set_notification();
                }
            }
        });
        t.start();
    }

    private void set_notification() {
        NotificationManager m_notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        
        SharedPreferences sp;
        prefs_lock.lock();
        sp = context.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
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
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, 0);
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
