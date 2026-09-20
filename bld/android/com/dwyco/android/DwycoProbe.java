package com.dwyco.android;
import com.dwyco.cdc32.dwybg;
import com.dwyco.config.DwycoApp;
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
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

public class DwycoProbe extends Worker {

    private Context context;
    private SocketLock prefs_lock;
    private boolean stop_poller;
    private static int wtf = 0;

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
        NotificationCompat.Builder m_builder;
        m_builder = new NotificationCompat.Builder(context, "dwycobg");
        m_builder.setContentTitle("Dwyco");
        m_builder.setAutoCancel(true);
        m_builder.setContentText("Waiting");
        m_builder.setOnlyAlertOnce(true);
        m_builder.setSmallIcon(DwycoApp.notification_icon());
        int def = NotificationCompat.DEFAULT_ALL;
        def = def & (~(NotificationCompat.DEFAULT_SOUND|NotificationCompat.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);
        Intent notintent = new Intent(context, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, PendingIntent.FLAG_IMMUTABLE);
        m_builder.setContentIntent(p);
        Notification not = m_builder.build();
        ForegroundInfo f = new ForegroundInfo(1, not);
        return f;
    }

    public void onStopped() {
        // note: the worker is being stopped, usually because the battery
        // saver has come on or the OS is reclaiming resources. in that
        // situation the native worker thread can be stuck holding all the
        // mutexes and the singleton lock socket. we can't reliably signal
        // it to exit (the battery saver interferes even with local socket
        // connects), so if it doesn't yield we kill the whole process.
        // leaving a zombie background process behind means the foreground
        // hangs forever in dwyco_request_singleton_lock() on the next start.
        stop_poller = true;
        catchLog("wtf " + String.valueOf(wtf));
        if(trySignalBackgroundThread())
        {
            // the background thread is still alive and stuck holding the
            // singleton lock socket. it will not exit on its own, and if
            // we leave it behind the foreground will hang forever on the
            // next start. kill the whole process so the OS reclaims the
            // socket and the next startup is clean.
            catchLog("work stopped - background stuck, exiting");
            System.exit(0);
        }
        else
        {
            // the background thread either exited cleanly or is already
            // gone, so the singleton lock socket is (or will be shortly)
            // free. no need to kill the process.
            catchLog("work stopped - background clear");
        }
    }

    private boolean trySignalBackgroundThread() {
        final int[] port = new int[1];
        if(!prefs_lock.timedLock(2000))
        {
            // couldn't get the prefs lock in time, treat the background
            // as stuck so the caller kills the process.
            catchLog("could not get prefs lock");
            return true;
        }
        try
        {
            SharedPreferences sp = context.getSharedPreferences(DwycoApp.shared_prefs, Context.MODE_PRIVATE);
            port[0] = sp.getInt("lockport", 4500);
            catchLog(String.valueOf(port[0]));
        }
        catch(Exception e)
        {
            catchLog(e.getMessage());
            return true;
        }
        finally
        {
            prefs_lock.release();
        }
        // give the native thread a chance to exit gracefully, but bound
        // the wait so we never hang in here forever. if the background
        // thread is stuck, we time out and the caller exits the process.
        final boolean[] stuck = new boolean[1];
        Thread t = new Thread(new Runnable() {
            public void run() {
                // don't use "getLoopback" in here, as it will try to use
                // ipv6 despite ipv4address being specified
                //Socket s = new Socket();
                //InetSocketAddress address=new InetSocketAddress(Inet4Address.getByName("127.0.0.1"), port);
                //s.connect(address, 1000);
                try
                {
                    LocalSocket s = new LocalSocket();
                    LocalSocketAddress a = new LocalSocketAddress("dwyco" + port[0]);
                    s.connect(a);
                    s.setSoTimeout(1000);
                    // read() returns -1 (eof) once the background thread
                    // closes the lock socket, i.e. it exited. if it never
                    // closes, this times out and we consider it stuck.
                    s.getInputStream().read();
                }
                catch(SocketTimeoutException e)
                {
                    catchLog("work stopped (timeout)");
                    stuck[0] = true;
                }
                catch(IOException e)
                {
                    // no listener or connect failed: bg already gone
                    catchLog("work stopped (already dead)");
                    catchLog(e.getMessage());
                }
                catch(Exception e)
                {
                    catchLog("some exception");
                    catchLog(e.getMessage());
                }
            }
        });
        t.setDaemon(true);
        t.start();
        try
        {
            // wait long enough for a clean handoff, but don't hang
            // forever in here.
            t.join(5000);
        }
        catch(InterruptedException e)
        {
        }
        if(t.isAlive())
        {
            // the signal thread is still blocked (likely the connect()
            // hung because the battery saver interferes with even local
            // sockets). treat the background as stuck.
            catchLog("signal attempt did not complete");
            stuck[0] = true;
        }
        catchLog("signal attempt done");
        return stuck[0];
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
        catchLog("wtf2 " + String.valueOf(wtf));
        // note: this is just in case we get a received message while
        // we are doing the sends. might want to consider just inhibiting
        // anything in the way of receive processing, but i don't think
        // this hurts anything.
        poller_thread();
        
        dwybg.dwyco_background_processing(port, DwycoApp.is_rando ? (DwycoApp.exit_if_outq_empty|DwycoApp.check_backup_once) : 0, sys_pfx, user_pfx, tmp_pfx, token);
        stop_poller = true;
        dwybg.dwyco_signal_msg_cond();
        catchLog("job end");
        catchLog("wtf3 " + String.valueOf(wtf));
        return Result.success();
    }

    private void poller_thread() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                catchLog("poll thread " + String.valueOf(wtf));
                ++wtf;
                while(!stop_poller)
                {
                    dwybg.dwyco_wait_msg_cond(0);
                    if(stop_poller) {
                        break;
                    }
                    set_notification();
                }
                catchLog("poll done");
                --wtf;
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
        NotificationCompat.Builder m_builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            if(quiet == 0) 
                m_builder = new NotificationCompat.Builder(context, "dwyco");
            else
                m_builder = new NotificationCompat.Builder(context, "dwyco-quiet");

        } else {
        m_builder = new NotificationCompat.Builder(context);
        int def = NotificationCompat.DEFAULT_ALL;
        if(quiet == 1)
            def = def & (~(NotificationCompat.DEFAULT_SOUND|NotificationCompat.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);
        }
        //m_builder.setSmallIcon(R.drawable.ic_stat_not_icon2);
        m_builder.setSmallIcon(DwycoApp.notification_icon());
        //m_builder.setColor(context.getResources().getColor(R.color.green));
        m_builder.setContentTitle(DwycoApp.content_title);
        m_builder.setAutoCancel(true);
        m_builder.setContentText(DwycoApp.new_available);
        m_builder.setOnlyAlertOnce(true);
        
        Intent notintent = new Intent(context, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(context, 1, notintent, PendingIntent.FLAG_IMMUTABLE);
        m_builder.setContentIntent(p);

        Notification not = m_builder.build();
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
