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
        final public static String lock_shared_prefs = "com.dwyco.rando.prefs";
	// name of shared preferences
        final public static String shared_prefs = "rando";
	// name of file provider for sharing files into the mediastore
	// warning! there is a bunch of other stupid glue in res/xml/provider_paths.xml
	// and  AndroidManifest.xml <provider... yada> needed to get this working.
	final public static String file_provider = "com.dwyco.rando.provider";
	// notification message to use when a msg has been downloaded in the
	// background and is available immediately
        final public static String new_received = "New rando received";
	// notification message used for fcm notification that msg is available
        final public static String new_available = "New rando";
	// title used on all notifications
        final public static String content_title = "Dwyco Rando";
	// tells whether the DwycoSender service should try to stay around
	// after the send queue is empty. Setting to true will improve 
	// msg receive performance on android 8+, but you also get an
	// annoying notification. if typical fcm performance of msg delivery in 15
	// or 20 minutes is not a problem, then set to false.
	// note: this isn't really used any more
        final public static boolean sender_sticky = false;

	// false means set the FLAG_SECURE on the app so direct screenshots are not permitted
	// (at least in some cases, seems a bit inconsistent depending on android version)
        final public static boolean allow_screenshots = true;

        final public static boolean is_rando = true;

	final public static boolean keep_screen_on = false;

	/// note: this is for telling the background running how to behave.
        // "or" these together. ca 12/2022, this will change for sure
        // depending on what i do with workmanager, etc.
        final public static int exit_if_outq_empty = 1;
        final public static int check_backup_once = 2;
	
}
