

package com.dwyco.phoo;
//import com.dwyco.phoo.R;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.util.Log;
import java.util.Calendar;
import android.net.ConnectivityManager;
import android.app.AlarmManager;
import android.net.NetworkInfo;
import android.content.BroadcastReceiver;
import android.os.Bundle;
import java.util.Iterator;
import java.util.Set;
import android.content.SharedPreferences;

public class Push_Notification extends BroadcastReceiver {

    private Context context;

    @Override
    public void onReceive(Context context, Intent intent) {
// TODO Auto-generated method stub
        this.context = context;
        catchLog("int " + intent);
        dumpIntent(intent);
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())
                || Intent.ACTION_MY_PACKAGE_REPLACED.equals(intent.getAction())) {

            SharedPreferences sp;

            sp = context.getSharedPreferences("phoo", Context.MODE_PRIVATE);
            int port = sp.getInt("lockport", 4500);
            String sys_pfx = sp.getString("sys_pfx", ".");
            String user_pfx = sp.getString("user_pfx", ".");
            String tmp_pfx = sp.getString("tmp_pfx", ".");
            Intent i = new Intent(context, Dwyco_Message.class);
            i.putExtra("lockport", port);
            i.putExtra("sys_pfx", sys_pfx);
            i.putExtra("user_pfx", user_pfx);
            i.putExtra("tmp_pfx", tmp_pfx);
            context.startService(i);

        } else {

            startService();
        }
    }

    public void startService() {
        /*
        if(false) {
            ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE );
            NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
            catchLog("ni " + activeNetInfo);
            boolean isConnected = activeNetInfo != null && activeNetInfo.isConnectedOrConnecting();

            Calendar cur_cal = Calendar.getInstance();
            cur_cal.setTimeInMillis(System.currentTimeMillis());
            Intent i = new Intent(context, Push_Notification_Service.class);
            PendingIntent pintent = PendingIntent.getService(context, 0, i, 0);
            AlarmManager alarm = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            isConnected = false;
            if (isConnected) {
                catchLog("connected " +isConnected);
                alarm.setRepeating(AlarmManager.RTC_WAKEUP, cur_cal.getTimeInMillis(), 1000 * 60, pintent);
            }
            else {
                catchLog("not connected " +isConnected);
                alarm.cancel(pintent);
            }
        }
    */
    }

    private static void catchLog(String log) {
        Log.d("dwyco_net_check", log);
    }

    private static void dumpIntent(Intent i) {

        Bundle bundle = i.getExtras();
        if (bundle != null) {
            Set<String> keys = bundle.keySet();
            Iterator<String> it = keys.iterator();
            catchLog("Dumping Intent start");
            while (it.hasNext()) {
                String key = it.next();
                catchLog("[" + key + "=" + bundle.get(key)+"]");
            }
            catchLog("Dumping Intent end");
        }
    }
}
