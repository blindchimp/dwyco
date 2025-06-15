package com.dwyco.android;
// note: this is needed for resources, but isn't
// really something i want here... WIP
//import com.dwyco.phoo.R;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageCapture;
import androidx.camera.core.ImageCaptureException;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.common.util.concurrent.ListenableFuture;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class CameraActivity extends AppCompatActivity {

    // A constant for the request code for camera permissions.
    private static final int REQUEST_CODE_PERMISSIONS = 10;
    // An array of required permissions.
    private static final String[] REQUIRED_PERMISSIONS = new String[]{Manifest.permission.CAMERA};
    // A tag for logging.
    private static final String TAG = "CameraActivity";
    // The format for the timestamp in the filename.
    private static final String FILENAME_FORMAT = "yyyy-MM-dd-HH-mm-ss-SSS";

    // UI elements
    private PreviewView viewFinder;
    private Button btnCapture, btnDone, btnUsePicture, btnTryAgain;
    private ImageView ivCapturedImage;
    private LinearLayout llPostCapture;


    // CameraX objects
    private ImageCapture imageCapture;
    private ExecutorService cameraExecutor;
    private File outputDirectory;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        // Initialize UI elements
        viewFinder = findViewById(R.id.viewFinder);
        btnCapture = findViewById(R.id.btn_capture);
        btnDone = findViewById(R.id.btn_done);
        btnUsePicture = findViewById(R.id.btn_use_picture);
        btnTryAgain = findViewById(R.id.btn_try_again);
        ivCapturedImage = findViewById(R.id.iv_captured_image);
        llPostCapture = findViewById(R.id.ll_post_capture);


        // Get the directory for storing captured images.
        outputDirectory = getOutputDirectory();
        // Create a new thread for camera operations.
        cameraExecutor = Executors.newSingleThreadExecutor();

        // Check for camera permissions.
        if (allPermissionsGranted()) {
            startCamera();
        } else {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS);
        }


        // Set up button click listeners
        btnCapture.setOnClickListener(v -> takePhoto());
        btnDone.setOnClickListener(v -> finish());
        btnTryAgain.setOnClickListener(v -> restartCameraPreview());
        // UsePicture button listener is set after a photo is taken
    }

    /**
     * Checks if all required permissions have been granted.
     * @return true if all permissions are granted, false otherwise.
     */
    private boolean allPermissionsGranted() {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    /**
     * Starts the camera preview.
     */
    private void startCamera() {
        // Get an instance of the ProcessCameraProvider.
        ListenableFuture<ProcessCameraProvider> cameraProviderFuture = ProcessCameraProvider.getInstance(this);

        cameraProviderFuture.addListener(() -> {
            try {
                // Get the camera provider.
                ProcessCameraProvider cameraProvider = cameraProviderFuture.get();

                // Set up the preview use case.
                Preview preview = new Preview.Builder().build();
                preview.setSurfaceProvider(viewFinder.getSurfaceProvider());

                // Set up the image capture use case.
                imageCapture = new ImageCapture.Builder().build();

                // Select the back camera as the default.
                CameraSelector cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA;

                // Unbind any existing use cases before rebinding.
                cameraProvider.unbindAll();

                // Bind the use cases to the camera.
                cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageCapture);

            } catch (Exception e) {
                Log.e(TAG, "Use case binding failed", e);
            }
        }, ContextCompat.getMainExecutor(this));
    }


    /**
     * Takes a photo and saves it to a file.
     */
    private void takePhoto() {
        if (imageCapture == null) {
            return;
        }

        // Create a file to save the image to.
        File photoFile = new File(
                outputDirectory,
                new SimpleDateFormat(FILENAME_FORMAT, Locale.US).format(System.currentTimeMillis()) + ".jpg");

        // Set up the output file options.
        ImageCapture.OutputFileOptions outputOptions = new ImageCapture.OutputFileOptions.Builder(photoFile).build();

        // Take the picture.
        imageCapture.takePicture(
                outputOptions, ContextCompat.getMainExecutor(this), new ImageCapture.OnImageSavedCallback() {
                    @Override
                    public void onImageSaved(@NonNull ImageCapture.OutputFileResults outputFileResults) {
                        // Get the URI of the saved image.
                        Uri savedUri = Uri.fromFile(photoFile);
                        String msg = "Photo capture succeeded: " + savedUri;
                        Log.d(TAG, msg);

                        // Show the captured image and the post-capture buttons.
                        ivCapturedImage.setImageURI(savedUri);
                        ivCapturedImage.setVisibility(View.VISIBLE);
                        viewFinder.setVisibility(View.GONE);
                        btnCapture.setVisibility(View.GONE);
                        llPostCapture.setVisibility(View.VISIBLE);

                        // Set up the "Use this Picture" button listener.
                        btnUsePicture.setOnClickListener(v -> {
                            // Return the URI of the saved image to the calling activity.
                            Intent resultIntent = new Intent();
                            resultIntent.putExtra("image_path", photoFile.getAbsolutePath());
                            setResult(RESULT_OK, resultIntent);
                            finish();
                        });
                    }

                    @Override
                    public void onError(@NonNull ImageCaptureException exception) {
                        Log.e(TAG, "Photo capture failed: " + exception.getMessage(), exception);
                    }
                });
    }

    /**
     * Restarts the camera preview.
     */
    private void restartCameraPreview() {
        // Make the viewfinder visible again
        viewFinder.setVisibility(View.VISIBLE);
        // Hide the captured image and post-capture buttons
        ivCapturedImage.setVisibility(View.GONE);
        llPostCapture.setVisibility(View.GONE);
        // Show the capture button again
        btnCapture.setVisibility(View.VISIBLE);
        // Restart the camera
        startCamera();
    }


    /**
     * Gets the directory for storing captured images.
     * @return The directory for storing captured images.
     */
    private File getOutputDirectory() {
        File[] mediaDirs = getExternalMediaDirs();
        File mediaDir = null;
        if (mediaDirs != null && mediaDirs.length > 0) {
            mediaDir = new File(mediaDirs[0], "PhooPics");
            mediaDir.mkdirs();
        }
        return mediaDir != null ? mediaDir : getFilesDir();
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                startCamera();
            } else {
                Toast.makeText(this, "Permissions not granted by the user.", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Shut down the camera executor.
        cameraExecutor.shutdown();
    }
}

