package com.dwyco.android;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class CameraActivity extends Activity {
    private static final String TAG = "CameraActivity";
    private static final int REQUEST_CAMERA_PERMISSION = 200;
    
    private SurfaceView surfaceView;
    private ImageView previewImageView;
    private Button captureButton;
    private Button doneButton;
    private Button switchCameraButton;
    private Button useThisPictureButton;
    private Button tryAgainButton;
    
    private CameraDevice cameraDevice;
    private CameraCaptureSession cameraCaptureSessions;
    private CaptureRequest.Builder captureRequestBuilder;
    private Size imageDimension;
    private ImageReader imageReader;
    private Handler backgroundHandler;
    private HandlerThread backgroundThread;
    
    private String cameraId;
    private boolean isBackCamera = true;
    private String[] cameraIds;
    private File currentImageFile;
    private boolean isPreviewMode = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setupUI();
        
        if (checkCameraPermission()) {
            initializeCamera();
        } else {
            requestCameraPermission();
        }
    }

    private void setupUI() {
        RelativeLayout mainLayout = new RelativeLayout(this);
        mainLayout.setBackgroundColor(0xFF000000);
        
        // Surface view for camera preview
        surfaceView = new SurfaceView(this);
        RelativeLayout.LayoutParams surfaceParams = new RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT,
            RelativeLayout.LayoutParams.MATCH_PARENT);
        surfaceView.setLayoutParams(surfaceParams);
        mainLayout.addView(surfaceView);
        
        // Image view for captured photo preview
        previewImageView = new ImageView(this);
        previewImageView.setLayoutParams(surfaceParams);
        previewImageView.setScaleType(ImageView.ScaleType.CENTER_CROP);
        previewImageView.setVisibility(View.GONE);
        mainLayout.addView(previewImageView);
        
        // Button container
        LinearLayout buttonContainer = new LinearLayout(this);
        buttonContainer.setOrientation(LinearLayout.HORIZONTAL);
        RelativeLayout.LayoutParams containerParams = new RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT,
            RelativeLayout.LayoutParams.WRAP_CONTENT);
        containerParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        containerParams.setMargins(20, 20, 20, 40);
        buttonContainer.setLayoutParams(containerParams);
        
        // Camera mode buttons
        doneButton = createButton("Done");
        captureButton = createButton("Capture");
        switchCameraButton = createButton("Switch");
        
        // Preview mode buttons
        tryAgainButton = createButton("Try Again");
        useThisPictureButton = createButton("Use This Picture");
        tryAgainButton.setVisibility(View.GONE);
        useThisPictureButton.setVisibility(View.GONE);
        
        buttonContainer.addView(doneButton);
        buttonContainer.addView(createSpacer());
        buttonContainer.addView(captureButton);
        buttonContainer.addView(createSpacer());
        buttonContainer.addView(switchCameraButton);
        buttonContainer.addView(tryAgainButton);
        buttonContainer.addView(createSpacer());
        buttonContainer.addView(useThisPictureButton);
        
        mainLayout.addView(buttonContainer);
        setContentView(mainLayout);
        
        setupButtonListeners();
    }
    
    private Button createButton(String text) {
        Button button = new Button(this);
        button.setText(text);
        button.setTextColor(0xFFFFFFFF);
        button.setBackgroundColor(0x88000000);
        button.setPadding(20, 15, 20, 15);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
            0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
        params.setMargins(5, 0, 5, 0);
        button.setLayoutParams(params);
        return button;
    }
    
    private View createSpacer() {
        View spacer = new View(this);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
            0, LinearLayout.LayoutParams.WRAP_CONTENT, 0.2f);
        spacer.setLayoutParams(params);
        return spacer;
    }
    
    private void setupButtonListeners() {
        captureButton.setOnClickListener(v -> takePicture());
        doneButton.setOnClickListener(v -> finish());
        switchCameraButton.setOnClickListener(v -> switchCamera());
        useThisPictureButton.setOnClickListener(v -> usePicture());
        tryAgainButton.setOnClickListener(v -> retryCapture());
    }
    
    private boolean checkCameraPermission() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) 
               == PackageManager.PERMISSION_GRANTED;
    }
    
    private void requestCameraPermission() {
        ActivityCompat.requestPermissions(this, 
            new String[]{Manifest.permission.CAMERA}, REQUEST_CAMERA_PERMISSION);
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, 
                                         @NonNull int[] grantResults) {
        if (requestCode == REQUEST_CAMERA_PERMISSION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initializeCamera();
            } else {
                Toast.makeText(this, "Camera permission required", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }
    
    private void initializeCamera() {
        CameraManager manager = (CameraManager) getSystemService(CAMERA_SERVICE);
        try {
            cameraIds = manager.getCameraIdList();
            if (cameraIds.length == 0) {
                Toast.makeText(this, "No cameras available", Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
            
            // Hide switch button if only one camera
            if (cameraIds.length <= 1) {
                switchCameraButton.setVisibility(View.GONE);
            }
            
            selectCamera();
            openCamera();
        } catch (CameraAccessException e) {
            Log.e(TAG, "Camera access exception", e);
        }
    }
    
    private void selectCamera() {
        CameraManager manager = (CameraManager) getSystemService(CAMERA_SERVICE);
        try {
            for (String id : cameraIds) {
                CameraCharacteristics characteristics = manager.getCameraCharacteristics(id);
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null) {
                    if (isBackCamera && facing == CameraCharacteristics.LENS_FACING_BACK) {
                        cameraId = id;
                        return;
                    } else if (!isBackCamera && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                        cameraId = id;
                        return;
                    }
                }
            }
            // Fallback to first available camera
            cameraId = cameraIds[0];
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error selecting camera", e);
            cameraId = cameraIds[0];
        }
    }
    
    private void openCamera() {
        CameraManager manager = (CameraManager) getSystemService(CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);
            StreamConfigurationMap map = characteristics.get(
                CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            
            if (map != null) {
                imageDimension = Collections.max(
                    Arrays.asList(map.getOutputSizes(ImageReader.NEWINSTANCE)), 
                    new CompareSizesByArea());
            }
            
            imageReader = ImageReader.newInstance(imageDimension.getWidth(), 
                imageDimension.getHeight(), ImageReader.getInstance().getFormat(), 1);
            imageReader.setOnImageAvailableListener(readerListener, backgroundHandler);
            
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) 
                != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            
            manager.openCamera(cameraId, stateCallback, null);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error opening camera", e);
        }
    }
    
    private final CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            cameraDevice = camera;
            createCameraPreview();
        }
        
        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            cameraDevice.close();
        }
        
        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            cameraDevice.close();
            cameraDevice = null;
        }
    };
    
    private void createCameraPreview() {
        try {
            SurfaceHolder holder = surfaceView.getHolder();
            Surface surface = holder.getSurface();
            
            captureRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            captureRequestBuilder.addTarget(surface);
            
            cameraDevice.createCaptureSession(Arrays.asList(surface, imageReader.getSurface()),
                new CameraCaptureSession.StateCallback() {
                    @Override
                    public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                        if (cameraDevice == null) return;
                        
                        cameraCaptureSessions = cameraCaptureSession;
                        updatePreview();
                    }
                    
                    @Override
                    public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
                        Toast.makeText(CameraActivity.this, "Configuration change", 
                            Toast.LENGTH_SHORT).show();
                    }
                }, null);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error creating camera preview", e);
        }
    }
    
    private void updatePreview() {
        if (cameraDevice == null) return;
        
        captureRequestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
        try {
            cameraCaptureSessions.setRepeatingRequest(captureRequestBuilder.build(), 
                null, backgroundHandler);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error updating preview", e);
        }
    }
    
    private void takePicture() {
        if (cameraDevice == null) return;
        
        try {
            CaptureRequest.Builder reader = cameraDevice.createCaptureRequest(
                CameraDevice.TEMPLATE_STILL_CAPTURE);
            reader.addTarget(imageReader.getSurface());
            reader.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
            
            // Create temp file
            File outputDir = getCacheDir();
            currentImageFile = File.createTempFile("camera_", ".jpg", outputDir);
            
            cameraCaptureSessions.capture(reader.build(), null, backgroundHandler);
        } catch (CameraAccessException | IOException e) {
            Log.e(TAG, "Error taking picture", e);
        }
    }
    
    private final ImageReader.OnImageAvailableListener readerListener = 
        new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireLatestImage();
            ByteBuffer buffer = image.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.remaining()];
            buffer.get(bytes);
            
            try (FileOutputStream output = new FileOutputStream(currentImageFile)) {
                output.write(bytes);
                runOnUiThread(() -> showPreview());
            } catch (IOException e) {
                Log.e(TAG, "Error saving image", e);
            } finally {
                image.close();
            }
        }
    };
    
    private void showPreview() {
        isPreviewMode = true;
        
        // Load and display the captured image
        Bitmap bitmap = BitmapFactory.decodeFile(currentImageFile.getAbsolutePath());
        if (bitmap != null) {
            // Rotate bitmap if needed (front camera is usually mirrored)
            if (!isBackCamera) {
                Matrix matrix = new Matrix();
                matrix.preScale(-1.0f, 1.0f);
                bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), 
                    bitmap.getHeight(), matrix, false);
            }
            previewImageView.setImageBitmap(bitmap);
        }
        
        // Switch UI to preview mode
        surfaceView.setVisibility(View.GONE);
        previewImageView.setVisibility(View.VISIBLE);
        captureButton.setVisibility(View.GONE);
        switchCameraButton.setVisibility(View.GONE);
        tryAgainButton.setVisibility(View.VISIBLE);
        useThisPictureButton.setVisibility(View.VISIBLE);
    }
    
    private void usePicture() {
        Intent result = new Intent();
        result.putExtra("image_path", currentImageFile.getAbsolutePath());
        setResult(RESULT_OK, result);
        finish();
    }
    
    private void retryCapture() {
        isPreviewMode = false;
        
        // Clean up current image
        if (currentImageFile != null && currentImageFile.exists()) {
            currentImageFile.delete();
            currentImageFile = null;
        }
        
        // Switch UI back to camera mode
        previewImageView.setVisibility(View.GONE);
        surfaceView.setVisibility(View.VISIBLE);
        tryAgainButton.setVisibility(View.GONE);
        useThisPictureButton.setVisibility(View.GONE);
        captureButton.setVisibility(View.VISIBLE);
        if (cameraIds.length > 1) {
            switchCameraButton.setVisibility(View.VISIBLE);
        }
    }
    
    private void switchCamera() {
        if (cameraIds.length <= 1) return;
        
        closeCamera();
        isBackCamera = !isBackCamera;
        selectCamera();
        openCamera();
    }
    
    private void closeCamera() {
        if (cameraCaptureSessions != null) {
            cameraCaptureSessions.close();
            cameraCaptureSessions = null;
        }
        if (cameraDevice != null) {
            cameraDevice.close();
            cameraDevice = null;
        }
        if (imageReader != null) {
            imageReader.close();
            imageReader = null;
        }
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        startBackgroundThread();
        if (surfaceView.getHolder().getSurface().isValid() && !isPreviewMode) {
            openCamera();
        }
    }
    
    @Override
    protected void onPause() {
        closeCamera();
        stopBackgroundThread();
        super.onPause();
    }
    
    @Override
    protected void onDestroy() {
        // Clean up any unused temp files
        if (currentImageFile != null && currentImageFile.exists() && isPreviewMode) {
            currentImageFile.delete();
        }
        super.onDestroy();
    }
    
    private void startBackgroundThread() {
        backgroundThread = new HandlerThread("Camera Background");
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
    }
    
    private void stopBackgroundThread() {
        if (backgroundThread != null) {
            backgroundThread.quitSafely();
            try {
                backgroundThread.join();
                backgroundThread = null;
                backgroundHandler = null;
            } catch (InterruptedException e) {
                Log.e(TAG, "Error stopping background thread", e);
            }
        }
    }
    
    @Override
    public void finish() {
        // Clean up temp file if not used
        if (currentImageFile != null && currentImageFile.exists()) {
            currentImageFile.delete();
        }
        super.finish();
    }
    
    private static class CompareSizesByArea implements Comparator<Size> {
        @Override
        public int compare(Size lhs, Size rhs) {
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                             (long) rhs.getWidth() * rhs.getHeight());
        }
    }
}