// this file is like a little resource file you can use to make minor
// customizations to the android java stuff for your app.
// this makes it possible to use common android java files
// for most apps without having to copy them around to your
// app folder.

package com.dwyco.android;
import com.dwyco.rando.R;

public class DwycoApp {
    public static int notification_icon() {
        return R.drawable.ic_stat_not_icon2;
        }

	// socket lock used for shared prefs locks
	public static String lock_shared_prefs = "com.dwyco.rando.prefs";
	// name of shared preferences
	public static String shared_prefs = "rando";
	// notification message to use when a msg has been downloaded in the
	// background and is available immediately
	public static String new_received = "New rando received";
	// notification message used for fcm notification that msg is available
	public static String new_available = "New rando";
	// title used on all notifications
	public static String content_title = "Dwyco Rando";
	// tells whether the DwycoSender service should try to stay around
	// after the send queue is empty. Setting to true will improve 
	// msg receive performance on android 8+, but you also get an
	// annoying notification. if typical fcm performance of msg delivery in 15
	// or 20 minutes is not a problem, then set to false.
	public static boolean sender_sticky = false;
	// false means set the FLAG_SECURE on the app so direct screenshots are not permitted
	// (at least in some cases, seems a bit inconsistent depending on android version)
	public static boolean allow_screenshots = true;
	

}
