package com.dwyco.android; // Adjust package name as per your project structure

import android.Manifest;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.Image;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.util.Size;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageCapture;
import androidx.camera.core.ImageCaptureException;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.common.util.concurrent.ListenableFuture;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class CameraActivity extends AppCompatActivity {

    private static final String TAG = "CameraActivity";
    private static final int REQUEST_CODE_PERMISSIONS = 10;
    private final String[] REQUIRED_PERMISSIONS = new String[]{Manifest.permission.CAMERA};

    private PreviewView previewView;
    private ImageView imageView;
    private Button captureButton, doneButton, switchCameraButton;
    private Button usePictureButton, tryAgainButton;
    private LinearLayout buttonLayoutTop, buttonLayoutBottom;
    private FrameLayout rootLayout;

    private ListenableFuture<ProcessCameraProvider> cameraProviderFuture;
    private ImageCapture imageCapture;
    private CameraSelector cameraSelector;
    private int lensFacing = CameraSelector.LENS_FACING_BACK;
    private ExecutorService cameraExecutor;

    private Uri capturedImageUri; // URI of the temporarily captured image

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Create the root FrameLayout to hold all views
        rootLayout = new FrameLayout(this);
        rootLayout.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        setContentView(rootLayout);

        // Initialize CameraX executor
        cameraExecutor = Executors.newSingleThreadExecutor();

        // 1. Setup Camera Preview View
        previewView = new PreviewView(this);
        previewView.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        rootLayout.addView(previewView);

        // 2. Setup ImageView for displaying captured photo
        imageView = new ImageView(this);
        imageView.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        imageView.setVisibility(View.GONE); // Initially hidden
        imageView.setScaleType(ImageView.ScaleType.FIT_CENTER); // Adjust as needed
        rootLayout.addView(imageView);

        // 3. Setup Top Button Layout (Capture, Done, Switch Camera)
        buttonLayoutTop = new LinearLayout(this);
        buttonLayoutTop.setOrientation(LinearLayout.HORIZONTAL);
        FrameLayout.LayoutParams topParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        topParams.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        topParams.setMargins(0, 0, 0, dpToPx(32)); // Margin from bottom
        buttonLayoutTop.setLayoutParams(topParams);
        buttonLayoutTop.setGravity(Gravity.CENTER_HORIZONTAL); // Center buttons horizontally
        rootLayout.addView(buttonLayoutTop);

        // Capture Button
        captureButton = new Button(this);
        captureButton.setText("Capture");
        LinearLayout.LayoutParams captureButtonParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        captureButtonParams.setMargins(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        captureButton.setLayoutParams(captureButtonParams);
        buttonLayoutTop.addView(captureButton);

        // Done Button
        doneButton = new Button(this);
        doneButton.setText("Done");
        LinearLayout.LayoutParams doneButtonParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        doneButtonParams.setMargins(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        doneButton.setLayoutParams(doneButtonParams);
        buttonLayoutTop.addView(doneButton);

        // Switch Camera Button (initially hidden if only one camera)
        switchCameraButton = new Button(this);
        switchCameraButton.setText("Switch Camera");
        LinearLayout.LayoutParams switchButtonParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        switchButtonParams.setMargins(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        switchCameraButton.setLayoutParams(switchButtonParams);
        buttonLayoutTop.addView(switchCameraButton);
        // Check for multiple cameras and hide if only one
        if (!hasMultipleCameras()) {
            switchCameraButton.setVisibility(View.GONE);
        }

        // 4. Setup Bottom Button Layout (Use this picture, Try again) - for post-capture
        buttonLayoutBottom = new LinearLayout(this);
        buttonLayoutBottom.setOrientation(LinearLayout.HORIZONTAL);
        FrameLayout.LayoutParams bottomParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        bottomParams.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        bottomParams.setMargins(0, 0, 0, dpToPx(32)); // Margin from bottom
        buttonLayoutBottom.setLayoutParams(bottomParams);
        buttonLayoutBottom.setGravity(Gravity.CENTER_HORIZONTAL); // Center buttons horizontally
        buttonLayoutBottom.setVisibility(View.GONE); // Initially hidden
        rootLayout.addView(buttonLayoutBottom);

        // Use this picture Button
        usePictureButton = new Button(this);
        usePictureButton.setText("Use this picture");
        LinearLayout.LayoutParams usePictureButtonParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        usePictureButtonParams.setMargins(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        usePictureButton.setLayoutParams(usePictureButtonParams);
        buttonLayoutBottom.addView(usePictureButton);

        // Try again Button
        tryAgainButton = new Button(this);
        tryAgainButton.setText("Try again");
        LinearLayout.LayoutParams tryAgainButtonParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        tryAgainButtonParams.setMargins(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        tryAgainButton.setLayoutParams(tryAgainButtonParams);
        buttonLayoutBottom.addView(tryAgainButton);


        // Check camera permissions
        if (allPermissionsGranted()) {
            startCamera();
        } else {
            ActivityCompat.requestPermissions(
                    this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS);
        }

        // Set up click listeners
        captureButton.setOnClickListener(v -> takePhoto());
        doneButton.setOnClickListener(v -> finishCameraActivity(null));
        switchCameraButton.setOnClickListener(v -> {
            lensFacing = (lensFacing == CameraSelector.LENS_FACING_BACK) ?
                    CameraSelector.LENS_FACING_FRONT : CameraSelector.LENS_FACING_BACK;
            startCamera(); // Rebind camera with new lens facing
        });

        usePictureButton.setOnClickListener(v -> finishCameraActivity(capturedImageUri));
        tryAgainButton.setOnClickListener(v -> showCameraPreview());
    }

    private boolean allPermissionsGranted() {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                startCamera();
            } else {
                Toast.makeText(this, "Permissions not granted by the user.", Toast.LENGTH_SHORT).show();
                finishCameraActivity(null); // Finish if permissions are not granted
            }
        }
    }

    private void startCamera() {
        cameraProviderFuture = ProcessCameraProvider.getInstance(this);
        cameraProviderFuture.addListener(() -> {
            try {
                ProcessCameraProvider cameraProvider = cameraProviderFuture.get();
                bindCameraUseCases(cameraProvider);
            } catch (Exception e) {
                Log.e(TAG, "Error starting camera: " + e.getMessage());
            }
        }, ContextCompat.getMainExecutor(this));
    }

    private void bindCameraUseCases(@NonNull ProcessCameraProvider cameraProvider) {
        cameraProvider.unbindAll(); // Unbind all existing use cases

        Preview preview = new Preview.Builder()
                .build();
        preview.setSurfaceProvider(previewView.getSurfaceProvider());

        imageCapture = new ImageCapture.Builder()
                .setCaptureMode(ImageCapture.CAPTURE_MODE_MINIMIZE_LATENCY)
                .setTargetResolution(new Size(1080, 1920)) // Example resolution, adjust as needed
                .build();

        cameraSelector = new CameraSelector.Builder()
                .requireLensFacing(lensFacing)
                .build();

        try {
            cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageCapture);
        } catch (Exception exc) {
            Log.e(TAG, "Use case binding failed", exc);
            Toast.makeText(this, "Failed to bind camera use cases: " + exc.getMessage(), Toast.LENGTH_LONG).show();
            finishCameraActivity(null);
        }
    }

    private void takePhoto() {
        if (imageCapture == null) {
            Log.e(TAG, "ImageCapture is null. Cannot take photo.");
            Toast.makeText(this, "Camera not ready.", Toast.LENGTH_SHORT).show();
            return;
        }

        // Create file to store the image
        final File photoFile; // Declared as final
        try {
            photoFile = createImageFile();
        } catch (IOException ex) {
            Log.e(TAG, "Error creating image file: " + ex.getMessage());
            Toast.makeText(this, "Error creating image file.", Toast.LENGTH_SHORT).show();
            return;
        }

        ImageCapture.OutputFileOptions outputFileOptions = new ImageCapture.OutputFileOptions.Builder(photoFile).build();

        imageCapture.takePicture(outputFileOptions, cameraExecutor, new ImageCapture.OnImageSavedCallback() {
            @Override
            public void onImageSaved(@NonNull ImageCapture.OutputFileResults outputFileResults) {
                capturedImageUri = outputFileResults.getSavedUri();
                if (capturedImageUri == null) {
                    // Fallback to file path if URI is null (e.g., for older Android versions or specific setups)
                    capturedImageUri = Uri.fromFile(photoFile); // photoFile is now final
                }
                Log.d(TAG, "Photo capture succeeded: " + capturedImageUri.toString());
                runOnUiThread(() -> showCapturedImage(capturedImageUri));
            }

            @Override
            public void onError(@NonNull ImageCaptureException exc) {
                Log.e(TAG, "Photo capture failed: " + exc.getMessage(), exc);
                Toast.makeText(CameraActivity.this, "Photo capture failed: " + exc.getMessage(), Toast.LENGTH_LONG).show();
                // If capture fails, return to camera preview
                runOnUiThread(CameraActivity.this::showCameraPreview);
            }
        });
    }


    /**
     * Creates a temporary image file in the app's cache directory.
     * This file will be deleted if the user cancels or tries again.
     *
     * @return The created File object.
     * @throws IOException If the file cannot be created.
     */
    private File createImageFile() throws IOException {
        // Create an image file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(System.currentTimeMillis());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = getExternalCacheDir(); // Use cache directory for temporary files
        if (storageDir == null) {
            storageDir = getCacheDir();
        }
        File image = File.createTempFile(
                imageFileName,  /* prefix */
                ".jpg",         /* suffix */
                storageDir      /* directory */
        );
        Log.d(TAG, "Created temporary image file: " + image.getAbsolutePath());
        return image;
    }


    /**
     * Displays the captured image and shows the "Use this picture" and "Try again" buttons.
     * Hides the camera preview and capture buttons.
     *
     * @param uri The URI of the captured image.
     */
    private void showCapturedImage(Uri uri) {
        if (uri == null) {
            Log.e(TAG, "Attempted to show null image URI.");
            Toast.makeText(this, "Error displaying image.", Toast.LENGTH_SHORT).show();
            showCameraPreview(); // Fallback to camera preview
            return;
        }

        try {
            // Decode the image from the URI
            Bitmap bitmap = BitmapFactory.decodeStream(getContentResolver().openInputStream(uri));
            if (bitmap != null) {
                // Rotate the bitmap if necessary (CameraX often captures in landscape even if device is portrait)
                // This is a common issue with CameraX and various devices.
                // A more robust solution might involve ImageAnalysis use case to get correct orientation
                // or checking EXIF data. For simplicity, we'll try a common rotation.
                // Note: ImageProxy provides rotationDegrees that can be used directly with ImageCapture
                // but if loading from URI, manual rotation might be needed depending on how it was saved.
                // For CameraX's `takePicture(OutputFileOptions...)`, the orientation should usually be correct.
                // However, if using `takePicture(ImageCapture.OnImageCapturedCallback)`, rotation needs to be handled manually.
                // Let's assume for `OutputFileOptions` it's handled, but keep this in mind for debugging.
                // If the image appears sideways, uncomment and adjust the rotation logic.
                /*
                Matrix matrix = new Matrix();
                matrix.postRotate(90); // Example rotation for portrait
                bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
                */

                imageView.setImageBitmap(bitmap);
                imageView.setVisibility(View.VISIBLE);
                previewView.setVisibility(View.GONE);

                buttonLayoutTop.setVisibility(View.GONE);
                buttonLayoutBottom.setVisibility(View.VISIBLE);
            } else {
                Log.e(TAG, "Failed to decode bitmap from URI: " + uri);
                Toast.makeText(this, "Failed to load captured image.", Toast.LENGTH_SHORT).show();
                showCameraPreview(); // Fallback to camera preview
            }
        } catch (Exception e) {
            Log.e(TAG, "Error displaying captured image: " + e.getMessage(), e);
            Toast.makeText(this, "Error displaying image.", Toast.LENGTH_SHORT).show();
            showCameraPreview(); // Fallback to camera preview
        }
    }

    /**
     * Hides the captured image and shows the camera preview and capture buttons.
     * Deletes the temporarily captured image.
     */
    private void showCameraPreview() {
        imageView.setImageDrawable(null); // Clear the image
        imageView.setVisibility(View.GONE);
        previewView.setVisibility(View.VISIBLE);

        buttonLayoutTop.setVisibility(View.VISIBLE);
        buttonLayoutBottom.setVisibility(View.GONE);

        // Delete the temporary image if it exists
        if (capturedImageUri != null) {
            File tempFile = new File(capturedImageUri.getPath());
            if (tempFile.exists() && tempFile.delete()) {
                Log.d(TAG, "Deleted temporary image: " + tempFile.getAbsolutePath());
            } else {
                Log.w(TAG, "Failed to delete temporary image: " + tempFile.getAbsolutePath());
            }
            capturedImageUri = null; // Clear the URI
        }
    }

    /**
     * Finishes the CameraActivity and returns the result to the calling activity.
     *
     * @param imageUri The URI of the image to return, or null if cancelled/failed.
     */
    private void finishCameraActivity(@Nullable Uri imageUri) {
        Intent resultIntent = new Intent();
        if (imageUri != null) {
            // Optionally, add the URI as data if the calling activity expects it via getData()
            // For file:// URIs, getPath() directly provides the absolute path.
            String imageAbsolutePath = imageUri.getPath();
            resultIntent.putExtra("image_path", imageAbsolutePath);
            Log.d(TAG, "Returning image absolute path: " + imageAbsolutePath);
            setResult(RESULT_OK, resultIntent);
        } else {
            setResult(RESULT_CANCELED, resultIntent);
            // Clean up any remaining temporary image if the activity is finished without accepting
            if (capturedImageUri != null) {
                File tempFile = new File(capturedImageUri.getPath());
                if (tempFile.exists() && tempFile.delete()) {
                    Log.d(TAG, "Deleted temporary image on cancellation: " + tempFile.getAbsolutePath());
                } else {
                    Log.w(TAG, "Failed to delete temporary image on cancellation: " + tempFile.getAbsolutePath());
                }
            }
        }
        finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        cameraExecutor.shutdown();
        // Ensure all temporary files are cleaned up on activity destruction
        if (capturedImageUri != null) {
            File tempFile = new File(capturedImageUri.getPath());
            if (tempFile.exists() && tempFile.delete()) {
                Log.d(TAG, "Deleted temporary image on onDestroy: " + tempFile.getAbsolutePath());
            } else {
                Log.w(TAG, "Failed to delete temporary image on onDestroy: " + tempFile.getAbsolutePath());
            }
            capturedImageUri = null;
        }
    }

    /**
     * Converts density-independent pixels (dp) to pixels (px).
     */
    private int dpToPx(int dp) {
        return (int) (dp * getResources().getDisplayMetrics().density + 0.5f);
    }

    /**
     * Checks if the device has multiple cameras (front and back).
     * This is a simple heuristic and might not cover all edge cases.
     */
    private boolean hasMultipleCameras() {
        try {
            ProcessCameraProvider cameraProvider = ProcessCameraProvider.getInstance(this).get();
            return cameraProvider.hasCamera(CameraSelector.DEFAULT_FRONT_CAMERA) &&
                   cameraProvider.hasCamera(CameraSelector.DEFAULT_BACK_CAMERA);
        } catch (Exception e) {
            Log.e(TAG, "Error checking camera count: " + e.getMessage());
            return false; // Assume single camera or error
        }
    }
}

