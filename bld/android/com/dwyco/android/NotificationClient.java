/****************************************************************************
**
****************************************************************************/

package com.dwyco.android;
import com.dwyco.cdc32.dwybg;
import org.qtproject.qt.android.bindings.QtActivity;
import org.qtproject.qt.android.bindings.QtApplication;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.os.Bundle;
import android.app.AlarmManager;
import java.util.Calendar;
import java.util.concurrent.TimeUnit;
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
import com.google.firebase.crashlytics.FirebaseCrashlytics;
import com.google.firebase.analytics.FirebaseAnalytics;
import com.google.firebase.messaging.FirebaseMessaging;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import android.content.ComponentName;
import androidx.core.content.FileProvider;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import android.os.ParcelFileDescriptor;
import android.content.ContentValues;
import java.io.IOException;
import java.io.File;

import androidx.work.*;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.core.content.ContextCompat;
//import androidx.activity.*;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;

// note: use notificationcompat stuff for older androids

public class NotificationClient extends QtActivity
{
    private static NotificationClient m_instance;
    public static String msg_count_url;
    private static SocketLock prefs_lock;
    public static int allow_notification = 1;
    private static FirebaseAnalytics mFirebaseAnalytics;
    private static String TAG = "notification_client";
    private static final int REQUEST_POST_NOTIFICATIONS = 1;
    private static final int REQUEST_EXTERNAL_STORAGE_PERMISSION = 2;

    public NotificationClient()
    {
        m_instance = this;
        prefs_lock = new SocketLock(DwycoApp.lock_shared_prefs);
    }

    @Override
    public void onCreate(Bundle state) {
        super.onCreate(state);
        
        if(!DwycoApp.allow_screenshots)
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
	if(DwycoApp.keep_screen_on)
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mFirebaseAnalytics = FirebaseAnalytics.getInstance(this);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {

                int importance = NotificationManager.IMPORTANCE_DEFAULT;
                NotificationChannel channel = new NotificationChannel("dwyco", "Dwyco Channel", importance);
                channel.setDescription("Dwyco message channel");
                // Register the channel with the system; you can't change the importance
                // or other notification behaviors after this
                NotificationManager notificationManager = getSystemService(NotificationManager.class);
                notificationManager.createNotificationChannel(channel);

                channel = new NotificationChannel("dwyco-quiet", "Quiet notification", NotificationManager.IMPORTANCE_LOW);
                channel.setDescription("Quiet message channel");
                channel.setShowBadge(false);
                notificationManager.createNotificationChannel(channel);

                channel = new NotificationChannel("dwycobg", "Waiting", NotificationManager.IMPORTANCE_LOW);
                channel.setDescription("Background send/receives");
                channel.setShowBadge(false);
                notificationManager.createNotificationChannel(channel);

            }

        // after a shitshow trying to get this working with the stupid-complicated
        // new api, an AI informed me that using the old-school simpler method was
        // doable. I'm not sure i need all the imports and other garbage i added
        // above while i was fiddling with using the new method. 
        // the only down side i see with this is that android will ask for
        // notification permissions twice before starting to ignore the request.
        // which is fine. otherwise i would have to save some state in the handler
        // below. not worth the troubles.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(this, new String[]{android.Manifest.permission.POST_NOTIFICATIONS}, REQUEST_POST_NOTIFICATIONS);
            }
        }

        // mostly AI generated, gemini 2.0. mods to fix
        // prompt: give me some android java code that will request the correct storage permission for writing an image to the Downloads directory on an android phone.
        // do not give me code for storing an image. i just want code for requesting the correct permission.
        // Android 10 (API 29) or higher: Assume MediaStore or other scoped storage mechanism is used.
            // No need to request WRITE_EXTERNAL_STORAGE permission if correctly using MediaStore.
            // **IMPORTANT**: Adjust this logic based on your actual MediaStore implementation.  It's possible you STILL need the permission,
            // particularly if interacting with files outside the scope of the MediaStore.
        
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
        // Check if we have write permission
        int permission = ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQUEST_EXTERNAL_STORAGE_PERMISSION
            );
        }
    }
    }


@Override
public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    if (requestCode == REQUEST_POST_NOTIFICATIONS) {
        if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            // Permission granted
        } else {
            // Permission denied
        }
    }
    if (requestCode == REQUEST_EXTERNAL_STORAGE_PERMISSION) {
        if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            // Permission granted.  Proceed with writing to external storage.
        } else {
            // Permission denied. Explain to the user that the feature is unavailable.
        }
    }
}


    @Override
    protected void onResume() {
        super.onResume();
	if(!DwycoApp.allow_screenshots)
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
        NotificationCompat.Builder m_builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        m_builder = new NotificationCompat.Builder(m_instance, "dwyco");
        } else {
        m_builder = new NotificationCompat.Builder(m_instance);
        }
        m_builder.setSmallIcon(DwycoApp.notification_icon());
        //m_builder.setColor(m_instance.getResources().getColor(R.color.green));
        m_builder.setContentTitle(DwycoApp.content_title);
        m_builder.setAutoCancel(true);
        m_builder.setContentText(DwycoApp.new_available);
        m_builder.setOnlyAlertOnce(true);
        SharedPreferences sp;
        prefs_lock.lock();
        sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
        int quiet = sp.getInt("quiet", 0);
        prefs_lock.release();
        int def = NotificationCompat.DEFAULT_ALL;
        if(quiet == 1)
            def = def & (~(NotificationCompat.DEFAULT_SOUND|NotificationCompat.DEFAULT_VIBRATE));
        m_builder.setDefaults(def);

        Intent notintent = new Intent(m_instance, NotificationClient.class);
        notintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent p = PendingIntent.getActivity(m_instance, 1, notintent, PendingIntent.FLAG_IMMUTABLE);
        m_builder.setContentIntent(p);

        Notification not = m_builder.build();
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
        SharedPreferences sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putString("url", s);
        pe.commit();
        prefs_lock.release();

    }

    public static void set_lastrun() {
        prefs_lock.lock();
        long secs = System.currentTimeMillis() / 1000; 
        SharedPreferences sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putLong("lastrun", secs);
        pe.commit();
        prefs_lock.release();

    }
    public static void set_quiet(int i) {
        prefs_lock.lock();
        SharedPreferences sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putInt("quiet", i);
        pe.commit();
        prefs_lock.release();
       // Crashlytics.getInstance().crash();
    }

    public static void set_service_params(int port, String sys_pfx, String user_pfx, String tmp_pfx) {
        prefs_lock.lock();
        SharedPreferences sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
        SharedPreferences.Editor pe = sp.edit();
        pe.putInt("lockport", port);
        pe.putString("sys_pfx", sys_pfx);
        pe.putString("user_pfx", user_pfx);
        pe.putString("tmp_pfx", tmp_pfx);
        pe.commit();
        prefs_lock.release();
    }

public static String get_token() {
    prefs_lock.lock();
    SharedPreferences sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
    String token = sp.getString("token", "notoken");
    prefs_lock.release();

    FirebaseMessaging.getInstance().getToken()
    .addOnCompleteListener(new OnCompleteListener<String>() {
        @Override
        public void onComplete(Task<String> task) {
          if (!task.isSuccessful()) {
            Log.w("TOKEN", "Fetching FCM registration token failed", task.getException());
            return;
          }

          // Get new FCM registration token
          String token = task.getResult();
          prefs_lock.lock();
          SharedPreferences sp;
          sp = m_instance.getSharedPreferences(DwycoApp.shared_prefs, MODE_PRIVATE);
          SharedPreferences.Editor pe = sp.edit();
          pe.putString("token", token);
          pe.commit();
          prefs_lock.release();
      
          Log.d("wrote token: ", token);
          // Log and toast
          //Log.d("TOKEN", token);
          
        }
    });

    return token;

    }

    public static void start_background() {
        // tested this all the way back to api 22, seems to work.
        Constraints constraints = new Constraints.Builder()
        .setRequiredNetworkType(NetworkType.CONNECTED)
        .build();

        if(DwycoApp.is_rando)
        {
            //OneTimeWorkRequest uploadWorkRequest = new OneTimeWorkRequest.Builder(DwycoProbe.class)
            //    .setConstraints(constraints)
            //   .build();
            //WorkManager.getInstance(m_instance).enqueueUniqueWork("upload_only", ExistingWorkPolicy.REPLACE, uploadWorkRequest);
            // note: this is a kluge right now, we can't do a backup if the main IU is alive in any way,
            // so we just kinda hope it gets killed off when this work is done so the backup gets
            // remade once in awhile. best way to fix this is to have the backup done in a separate
            // work thing. the reason i don't fix it is because i'm unsure how useful the backup
            // feature actually is for rando.
            PeriodicWorkRequest uploadWorkRequest = new PeriodicWorkRequest.Builder(DwycoProbe.class, 12, TimeUnit.HOURS)
                .setConstraints(constraints)
                .build();
            
            WorkManager.getInstance(m_instance)
                .enqueueUniquePeriodicWork("upload_only", 
                ExistingPeriodicWorkPolicy.CANCEL_AND_REENQUEUE, uploadWorkRequest);
        }
        else
        {
        
            PeriodicWorkRequest uploadWorkRequest = new PeriodicWorkRequest.Builder(DwycoProbe.class, 1, TimeUnit.HOURS)
                .setConstraints(constraints)
                .build();
            
            WorkManager.getInstance(m_instance)
                .enqueueUniquePeriodicWork("upload_only", 
                ExistingPeriodicWorkPolicy.CANCEL_AND_REENQUEUE, uploadWorkRequest);
        }
    }

public static void log_event() {
    Bundle bundle = new Bundle();
    bundle.putString(FirebaseAnalytics.Param.METHOD, "regular");
    mFirebaseAnalytics.logEvent(FirebaseAnalytics.Event.LOGIN, bundle);

    }

public static void log_event2(String name, String method) {
    Bundle bundle = new Bundle();
    bundle.putString(FirebaseAnalytics.Param.METHOD, method);
    mFirebaseAnalytics.logEvent(name, bundle);

    }

public static void set_user_property(String name, String value) {
    mFirebaseAnalytics.setUserProperty(name, value);
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
                //String filePath = getPath(getApplicationContext(), data.getData());
                String filePath = FileUtils.getRealPath(getApplicationContext(), data.getData());
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

    public static  void notifyMediaStoreScanner(String filepath) throws IOException {

        Context mContext = m_instance.getApplicationContext();
        Uri cacheImageUri;
        File f = new File(filepath);
        if(!f.exists()) {
            Log.e(TAG, "file doesn't exist");
            throw new IOException("file to share doesn't exist");
        }
        cacheImageUri = FileProvider.getUriForFile(mContext, DwycoApp.file_provider, f);
        Uri newImageUri = createImageInMediaStore(mContext, cacheImageUri);
        ContentResolver resolver = mContext
                .getContentResolver();
        FileInputStream input ;
        FileOutputStream outputStream ;
        try (ParcelFileDescriptor pfd = resolver
                .openFileDescriptor(newImageUri, "w", null)) {
            // Write data into the pending image.
            try(ParcelFileDescriptor  pfdInput  = resolver.openFileDescriptor(cacheImageUri , "r",null)){

                outputStream =new FileOutputStream(pfd.getFileDescriptor());
                input = new FileInputStream(pfdInput.getFileDescriptor());
                int  readResponse;
                do {
                    byte[] bytes = new byte[1024];
                    readResponse = input.read(bytes);
                    outputStream.write(bytes);
                }while (readResponse !=-1);
                input.close();
                outputStream.close();
            } catch (IOException e) {
                Log.e(TAG, e.getMessage());
                throw  new IOException("Error saving image ");
            }

        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
            throw  new IOException("Error saving image ");
        }
    }


    private static Uri createImageInMediaStore(Context mContext, Uri cacheImageUri){
        if (mContext== null)
            throw new IllegalArgumentException("mContext can not be null");
        if(cacheImageUri == null)
            throw new IllegalArgumentException("cacheImageUri can not be null");

        String imageName = cacheImageUri.getLastPathSegment();
        ContentResolver contentResolver = mContext.getContentResolver();
        Uri imagesCollection;

        if(Build.VERSION.SDK_INT  <= Build.VERSION_CODES.P){
            imagesCollection = MediaStore.Images.Media.getContentUri(MediaStore.VOLUME_EXTERNAL);
        }else{
            imagesCollection = MediaStore.Images.Media.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY);
        }

        ContentValues newImageContentValues = new ContentValues();
        newImageContentValues.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, imageName);
        return  contentResolver.insert(imagesCollection, newImageContentValues);
    }

    private static void catchLog(String log) {
        Log.d("dwyco_notfication", log);
    }

}


