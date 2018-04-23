package com.dwyco.phoo;

import android.util.Log;
import android.content.Context;
import com.google.firebase.iid.FirebaseInstanceId;
import com.google.firebase.iid.FirebaseInstanceIdService;
import android.content.SharedPreferences;


public class MyFirebaseInstanceIDService extends FirebaseInstanceIdService {

    private static final String TAG = "MyFirebaseIIDService";
    private static SocketLock prefs_lock;
    private static Context context;
    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        context = this;
        prefs_lock = new SocketLock("com.dwyco.phoo.prefs");
        Log.d(TAG, "firebase Service got created");
        // in case this service is started afresh in a new process,
        // we need this stuff in order to write the token into the
        // right place. if this is invoked in a new thread in
        // the existing foreground process, both of these have already
        // been loaded, so it shouldn't load a new one.
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("dwyco_jni");
    }
    /**
     * Called if InstanceID token is updated. This may occur if the security of
     * the previous token had been compromised. Note that this is called when the InstanceID token
     * is initially generated so this is where you would retrieve the token.
     */
    // [START refresh_token]
    @Override
    public void onTokenRefresh() {
        // Get updated InstanceID token.
        String refreshedToken = FirebaseInstanceId.getInstance().getToken();
        Log.d(TAG, "Refreshed token: " + refreshedToken);

        // If you want to send messages to this application instance or
        // manage this apps subscriptions on the server side, send the
        // Instance ID token to your app server.
        sendRegistrationToServer(refreshedToken);
    }
    // [END refresh_token]

    /**
     * Persist token to third-party servers.
     *
     * Modify this method to associate the user's FCM InstanceID token with any server-side account
     * maintained by your application.
     *
     * @param token The new token.
     */
    private void sendRegistrationToServer(String token) {
// this really doesn't work, need to just stick the token into the
// preferences and then deal with it in the main thread
        prefs_lock.lock();
        SharedPreferences sp;
        sp = getSharedPreferences("phoo", MODE_PRIVATE);
        String sys_pfx;
        String user_pfx;
        String tmp_pfx;
        sys_pfx = sp.getString("sys_pfx", ".");
        user_pfx = sp.getString("user_pfx", ".");
        tmp_pfx = sp.getString("tmp_pfx", ".");
        Log.d(TAG, sys_pfx);
        Log.d(TAG, user_pfx);
        Log.d(TAG, tmp_pfx);
        while(!sp.contains("sys_pfx") || !sp.contains("user_pfx") || !sp.contains("tmp_pfx")) {
            prefs_lock.release();
            Log.d(TAG, "waiting for init");
            try {
                Thread.sleep(1000);

            } catch(InterruptedException ex) {
                Thread.currentThread().interrupt();
            }
            prefs_lock.lock();
        }

        prefs_lock.release();

        Log.d(TAG, "wrote token: " + token);
        dwybg.dwyco_write_token(token);
    }
}
