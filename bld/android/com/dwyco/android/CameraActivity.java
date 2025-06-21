
// CameraActivity.java - Create this as a separate file
package com.dwyco.android;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class CameraActivity extends Activity implements SurfaceHolder.Callback {
    private static final String TAG = "CameraActivity";
    private static final int CAMERA_PERMISSION_REQUEST = 100;
    
    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }
    
    private CameraDevice cameraDevice;
    private CameraCaptureSession captureSession;
    private CaptureRequest.Builder previewRequestBuilder;
    private ImageReader imageReader;
    private HandlerThread backgroundThread;
    private Handler backgroundHandler;
    
    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private ImageView imagePreview;
    private LinearLayout cameraControls;
    private LinearLayout previewControls;
    private Button captureButton;
    private Button doneButton;
    private Button useThisPictureButton;
    private Button tryAgainButton;
    
    private String currentImagePath;
    private String tempImagePath;
    private boolean isPreviewMode = false;
    private String cameraId;
    private Size imageDimension;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Create layout programmatically for simplicity
        createLayout();
        
        // Check camera permission
        if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.CAMERA) 
            != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, 
                new String[]{android.Manifest.permission.CAMERA}, 
                CAMERA_PERMISSION_REQUEST);
        } else {
            initializeCamera();
        }
        
        setupButtonListeners();
    }
    
    private void createLayout() {
        LinearLayout mainLayout = new LinearLayout(this);
        mainLayout.setOrientation(LinearLayout.VERTICAL);
        mainLayout.setBackgroundColor(0xFF000000);
        
        // Surface view for camera preview
        surfaceView = new SurfaceView(this);
        LinearLayout.LayoutParams surfaceParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 0, 1.0f);
        surfaceView.setLayoutParams(surfaceParams);
        mainLayout.addView(surfaceView);
        
        // Image view for captured photo preview
        imagePreview = new ImageView(this);
        imagePreview.setLayoutParams(surfaceParams);
        imagePreview.setScaleType(ImageView.ScaleType.CENTER_CROP);
        imagePreview.setVisibility(View.GONE);
        mainLayout.addView(imagePreview);
        
        // Camera controls
        cameraControls = new LinearLayout(this);
        cameraControls.setOrientation(LinearLayout.HORIZONTAL);
        cameraControls.setBackgroundColor(0xFF333333);
        LinearLayout.LayoutParams controlsParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        cameraControls.setLayoutParams(controlsParams);
        
        captureButton = new Button(this);
        captureButton.setText("Capture");
        captureButton.setLayoutParams(new LinearLayout.LayoutParams(0, 
            LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f));
        cameraControls.addView(captureButton);
        
        doneButton = new Button(this);
        doneButton.setText("Done");
        doneButton.setLayoutParams(new LinearLayout.LayoutParams(0, 
            LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f));
        cameraControls.addView(doneButton);
        
        mainLayout.addView(cameraControls);
        
        // Preview controls
        previewControls = new LinearLayout(this);
        previewControls.setOrientation(LinearLayout.HORIZONTAL);
        previewControls.setBackgroundColor(0xFF333333);
        previewControls.setLayoutParams(controlsParams);
        previewControls.setVisibility(View.GONE);
        
        useThisPictureButton = new Button(this);
        useThisPictureButton.setText("Use This Picture");
        useThisPictureButton.setLayoutParams(new LinearLayout.LayoutParams(0, 
            LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f));
        previewControls.addView(useThisPictureButton);
        
        tryAgainButton = new Button(this);
        tryAgainButton.setText("Try Again");
        tryAgainButton.setLayoutParams(new LinearLayout.LayoutParams(0, 
            LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f));
        previewControls.addView(tryAgainButton);
        
        mainLayout.addView(previewControls);
        
        setContentView(mainLayout);
    }
    
    private void setupButtonListeners() {
        captureButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                captureImage();
            }
        });
        
        doneButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cleanupAndExit();
            }
        });
        
        useThisPictureButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                returnImagePath();
            }
        });
        
        tryAgainButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchToCameraMode();
            }
        });
    }
    
    private void initializeCamera() {
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
        startBackgroundThread();
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
    public void surfaceCreated(SurfaceHolder holder) {
        openCamera();
    }
    
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // Camera2 API handles surface changes automatically
    }
    
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        closeCamera();
    }
    
    private void openCamera() {
        CameraManager manager = (CameraManager) getSystemService(CAMERA_SERVICE);
        try {
            cameraId = manager.getCameraIdList()[0];
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);
            StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            
            // Get optimal image size
            Size[] imageSizes = map.getOutputSizes(ImageFormat.JPEG);
            imageDimension = getOptimalSize(imageSizes, 1920, 1080);
            
            // Set up ImageReader
            imageReader = ImageReader.newInstance(imageDimension.getWidth(), 
                imageDimension.getHeight(), ImageFormat.JPEG, 1);
            imageReader.setOnImageAvailableListener(onImageAvailableListener, backgroundHandler);
            
            if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.CAMERA) 
                != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            
            manager.openCamera(cameraId, stateCallback, null);
            
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error opening camera", e);
            Toast.makeText(this, "Error accessing camera", Toast.LENGTH_SHORT).show();
            finish();
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
            cameraDevice = null;
        }
        
        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            cameraDevice.close();
            cameraDevice = null;
            Log.e(TAG, "Camera error: " + error);
            finish();
        }
    };
    
    private void createCameraPreview() {
        try {
            Surface surface = surfaceHolder.getSurface();
            previewRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            previewRequestBuilder.addTarget(surface);
            
            cameraDevice.createCaptureSession(Arrays.asList(surface, imageReader.getSurface()),
                new CameraCaptureSession.StateCallback() {
                    @Override
                    public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                        if (cameraDevice == null) return;
                        
                        captureSession = cameraCaptureSession;
                        updatePreview();
                    }
                    
                    @Override
                    public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
                        Toast.makeText(CameraActivity.this, "Configuration change", Toast.LENGTH_SHORT).show();
                    }
                }, null);
                
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error creating camera preview", e);
        }
    }
    
    private void updatePreview() {
        if (cameraDevice == null) return;
        
        try {
            previewRequestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
            captureSession.setRepeatingRequest(previewRequestBuilder.build(), null, backgroundHandler);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error updating preview", e);
        }
    }
    
    private Size getOptimalSize(Size[] sizes, int maxWidth, int maxHeight) {
        if (sizes == null) return new Size(640, 480);
        
        for (Size size : sizes) {
            if (size.getWidth() <= maxWidth && size.getHeight() <= maxHeight) {
                return size;
            }
        }
        return sizes[0];
    }
    
    private void captureImage() {
        if (cameraDevice == null) return;
        
        try {
            CaptureRequest.Builder captureBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(imageReader.getSurface());
            
            // Auto focus
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            
            // Orientation
            int rotation = getWindowManager().getDefaultDisplay().getRotation();
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, ORIENTATIONS.get(rotation));
            
            CameraCaptureSession.CaptureCallback captureCallback = new CameraCaptureSession.CaptureCallback() {
                @Override
                public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                             @NonNull CaptureRequest request,
                                             @NonNull TotalCaptureResult result) {
                    super.onCaptureCompleted(session, request, result);
                    runOnUiThread(() -> Toast.makeText(CameraActivity.this, "Image captured", Toast.LENGTH_SHORT).show());
                }
            };
            
            captureSession.capture(captureBuilder.build(), captureCallback, backgroundHandler);
            
        } catch (CameraAccessException e) {
            Log.e(TAG, "Error capturing image", e);
        }
    }
    
    private final ImageReader.OnImageAvailableListener onImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireLatestImage();
            if (image == null) return;
            
            backgroundHandler.post(new ImageSaver(image));
        }
    };
    
    private class ImageSaver implements Runnable {
        private final Image image;
        
        ImageSaver(Image image) {
            this.image = image;
        }
        
        @Override
        public void run() {
            try {
                // Create temporary image file in cache directory
                String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault()).format(new Date());
                String imageFileName = "TEMP_IMG_" + timeStamp + ".jpg";
                
                File cacheDir = getCacheDir();
                File imageFile = new File(cacheDir, imageFileName);
                tempImagePath = imageFile.getAbsolutePath();
                
                ByteBuffer buffer = image.getPlanes()[0].getBuffer();
                byte[] bytes = new byte[buffer.remaining()];
                buffer.get(bytes);
                
                try (FileOutputStream output = new FileOutputStream(imageFile)) {
                    output.write(bytes);
                }
                
                // Display the captured image on UI thread
                runOnUiThread(() -> displayCapturedImage());
                
            } catch (IOException e) {
                Log.e(TAG, "Error saving image", e);
                runOnUiThread(() -> Toast.makeText(CameraActivity.this, "Error saving image", Toast.LENGTH_SHORT).show());
            } finally {
                image.close();
            }
        }
    }
    
    private void displayCapturedImage() {
        try {
            Bitmap bitmap = BitmapFactory.decodeFile(tempImagePath);
            if (bitmap != null) {
                imagePreview.setImageBitmap(bitmap);
                switchToPreviewMode();
            }
        } catch (Exception e) {
            Log.e(TAG, "Error displaying image: " + e.getMessage());
        }
    }
    
    private void switchToPreviewMode() {
        isPreviewMode = true;
        surfaceView.setVisibility(View.GONE);
        imagePreview.setVisibility(View.VISIBLE);
        cameraControls.setVisibility(View.GONE);
        previewControls.setVisibility(View.VISIBLE);
    }
    
    private void switchToCameraMode() {
        isPreviewMode = false;
        surfaceView.setVisibility(View.VISIBLE);
        imagePreview.setVisibility(View.GONE);
        cameraControls.setVisibility(View.VISIBLE);
        previewControls.setVisibility(View.GONE);
        
        // Clean up the temporary image
        cleanupTempImage();
    }
    
    private void returnImagePath() {
        if (tempImagePath != null) {
            // Move image from temp to permanent location
            try {
                String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault()).format(new Date());
                String imageFileName = "IMG_" + timeStamp + ".jpg";
                
                File storageDir = new File(getExternalFilesDir(Environment.DIRECTORY_PICTURES), "Camera");
                if (!storageDir.exists() && !storageDir.mkdirs()) {
                    Log.e(TAG, "Failed to create directory");
                    setResult(RESULT_CANCELED);
                    finish();
                    return;
                }
                
                File permanentFile = new File(storageDir, imageFileName);
                File tempFile = new File(tempImagePath);
                
                if (tempFile.renameTo(permanentFile)) {
                    currentImagePath = permanentFile.getAbsolutePath();
                    tempImagePath = null; // Clear temp path since file was moved
                    
                    Intent resultIntent = new Intent();
                    resultIntent.putExtra("image_path", currentImagePath);
                    setResult(RESULT_OK, resultIntent);
                } else {
                    Log.e(TAG, "Failed to move temp file to permanent location");
                    setResult(RESULT_CANCELED);
                }
            } catch (Exception e) {
                Log.e(TAG, "Error moving image file", e);
                setResult(RESULT_CANCELED);
            }
        } else {
            setResult(RESULT_CANCELED);
        }
        finish();
    }
    
    private void cleanupTempImage() {
        if (tempImagePath != null) {
            File tempFile = new File(tempImagePath);
            if (tempFile.exists() && tempFile.delete()) {
                Log.d(TAG, "Temporary image deleted: " + tempImagePath);
            }
            tempImagePath = null;
        }
    }
    
    private void cleanupAndExit() {
        cleanupTempImage();
        setResult(RESULT_CANCELED);
        finish();
    }
    
    private void closeCamera() {
        if (captureSession != null) {
            captureSession.close();
            captureSession = null;
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
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == CAMERA_PERMISSION_REQUEST) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initializeCamera();
            } else {
                Toast.makeText(this, "Camera permission required", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        startBackgroundThread();
        if (/*surfaceView.isAvailable() &&*/ !isPreviewMode) {
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
        cleanupTempImage();
        closeCamera();
        stopBackgroundThread();
        super.onDestroy();
    }
    
    @Override
    public void onBackPressed() {
        if (isPreviewMode) {
            switchToCameraMode();
        } else {
            cleanupAndExit();
        }
    }
}