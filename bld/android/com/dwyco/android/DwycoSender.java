package com.dwyco.android;
import com.dwyco.cdc32.dwybg;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.app.Service;
import android.os.Binder;
import android.os.IBinder;
import android.app.IntentService;
import android.content.SharedPreferences;

import android.os.Build;
import android.os.Build.VERSION;


public class DwycoSender extends Service {

    private static Context context;
    private static String[] local_files = {};
    private static SocketLock prefs_lock;

    public DwycoSender() {
        //super("DwycoSender");
        prefs_lock = new SocketLock(DwycoApp.lock_shared_prefs);

    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        catchLog("DwycoSender Service got created");
        context = this;
        System.loadLibrary("c++_shared");
        System.loadLibrary("dwyco_jni");
    }

    /**
     * Unless you provide binding for your service, you don't need to implement this
     * method, because the default implementation returns null.
     * @see android.app.Service#onBind
     */
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private boolean prefs_ok() {
        boolean ret;
        prefs_lock.lock();
        SharedPreferences sp;
        sp = context.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
        if(!sp.contains("lockport") || !sp.contains("sys_pfx") ||
            !sp.contains("user_pfx") || !sp.contains("tmp_pfx"))
            ret = false;
        ret = true;
        prefs_lock.release();
        return ret;
    }


    @Override
    public int onStartCommand(final Intent intent, int flags, int startId) {
        if(!prefs_ok()) {
            // something is really wrong, maybe user cleared data
            // and we got started before it could be regenerated or something
            catchLog("dwycoSend prefs hosed");
            System.exit(0);
        }
        Thread t = new Thread(new Runnable() {
            public void run() {
        catchLog("dwycoSend handle intent");
        prefs_lock.lock();
        SharedPreferences sp;
        int port;
        String sys_pfx;
        String user_pfx;
        String tmp_pfx;
        String token;

        if(intent == null) {
            // restart, fetch the values from preferences

            sp = context.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
            port = sp.getInt("lockport", 4500);
            sys_pfx = sp.getString("sys_pfx", ".");
            user_pfx = sp.getString("user_pfx", ".");
            tmp_pfx = sp.getString("tmp_pfx", ".");
            token = sp.getString("token", "notoken");
        } else {
            port = intent.getIntExtra("lockport", 4500);
            sys_pfx = intent.getStringExtra("sys_pfx");
            user_pfx = intent.getStringExtra("user_pfx");
            tmp_pfx = intent.getStringExtra("tmp_pfx");
            token = intent.getStringExtra("token");

            // write it back out for later if we need to restart
            sp = context.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
            SharedPreferences.Editor pe = sp.edit();
            pe.putInt("lockport", port);
            pe.putString("sys_pfx", sys_pfx);
            pe.putString("user_pfx", user_pfx);
            pe.putString("tmp_pfx", tmp_pfx);
            pe.putString("token", token);
            pe.commit();
        }
    prefs_lock.release();
        catchLog(sys_pfx);
        catchLog(user_pfx);
        catchLog(tmp_pfx);
        catchLog(String.valueOf(port));
        catchLog(token);
        poller_thread();
        
        set_notification();
        dwybg.dwyco_background_processing(port, DwycoApp.sender_sticky ? 0 : 1, sys_pfx, user_pfx, tmp_pfx, token);
        stopForeground(STOP_FOREGROUND_REMOVE);
        catchLog("send end");
        System.exit(0);
            }
        }
        );
        t.start();
        return DwycoApp.sender_sticky ? START_STICKY : START_NOT_STICKY;
        //System.exit(0);

    }
    @Override
    public void onDestroy() {
        super.onDestroy();
        catchLog("DESTROY");
        System.exit(0);
    }

    private void poller_thread() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                catchLog("poll thread");
                // first signal is just indicator that init is done
                dwybg.dwyco_wait_msg_cond(0);
                while(true)
                {
                    dwybg.dwyco_wait_msg_cond(0);
                    set_msg_notification();
                }
            }
        });
        t.start();
    }

    private void set_msg_notification() {
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
        m_builder.setSmallIcon(DwycoApp.notification_icon());
        //m_builder.setColor(context.getResources().getColor(R.color.green));
        m_builder.setContentTitle(DwycoApp.content_title);
        m_builder.setAutoCancel(true);
        m_builder.setContentText(DwycoApp.new_received);
        m_builder.setOnlyAlertOnce(true);
        
        Intent notintent = new Intent(context, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, PendingIntent.FLAG_IMMUTABLE);
        m_builder.setContentIntent(p);

        Notification not = m_builder.getNotification();
        m_notificationManager.notify(1, not);
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
            
                m_builder = new Notification.Builder(context, "dwycobg");
            

        } else {
        m_builder = new Notification.Builder(context);
        int def = Notification.DEFAULT_ALL;
        if(quiet == 1)
            def = def & (~(Notification.DEFAULT_SOUND|Notification.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);
        }
        m_builder.setSmallIcon(DwycoApp.notification_icon());
        //m_builder.setColor(context.getResources().getColor(R.color.green));
        m_builder.setContentTitle(DwycoApp.content_title);
        m_builder.setAutoCancel(true);
	if(DwycoApp.sender_sticky)
		m_builder.setContentText("Waiting");
	else
		m_builder.setContentText("Uploading");

        m_builder.setOnlyAlertOnce(true);
        
        Intent notintent = new Intent(context, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, PendingIntent.FLAG_IMMUTABLE);
        m_builder.setContentIntent(p);

        Notification not = m_builder.getNotification();
        startForeground(1, not);
    }
    

    private void clear_notification() {
        NotificationManager m_notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        m_notificationManager.cancelAll();
    }

    private void catchLog(String log) {
        Log.d("DwycoSender", log);


    }

}
