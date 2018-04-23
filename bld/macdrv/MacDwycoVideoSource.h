
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
//  MacDwycoVideoSource.h
//  DwycoVideoTest
//
//  Created by Christopher Corbell on 8/26/09.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import <Quartz/Quartz.h>
#import "MacDwycoVideoSourceDelegate.h"

/**
 * A Mac OS X video capture implementation designed for integration with the
 * dwyco DLL's video capture callbacks and the MacDwyco interface wrapper.
 *
 * <p>This class uses QTCaptureDevice and related QTKit classes to obtain
 * video frames.  It requires QuickTime 7.2 or later and Mac OS 10.4 or later.
 *
 * <p>A design assumption of this class is that dropped frames may be ignored
 * if the video device is capturing frames faster than the application needs them.
 *
 * <p><b>Thread Safety:</b></p>
 *
 * <p>This class is implemented with optional thread safety around the last
 * frame of image data captured (stored in the member lastCapturedImageBuffer).
 * By default when a client object invokes bitmapDataFromBitmapImageRep the
 * last frame buffer will be retrieved and nullified as a class member in
 * a thread-safe manner (protected by lastCapturedImageBufferMutex).  Setting
 * the new lastCapturedImageBuffer from the capture callback also honors
 * this mutex.
 *
 * <p>In most cases it is probably <i>not</i> necessary for the application
 * to use a worker thread to obtain video frames.  An NSTimer on the main
 * thread should be capable of driving the frame retrieval for the dwyco
 * library (by invoking dwyco_service_channels) at a rate sufficient for
 * good video display.
 *
 */
@interface MacDwycoVideoSource : NSObject<MacDwycoVideoSourceDelegate> {
    QTCaptureSession * captureSession;
    QTCaptureDevice * captureDevice;
    QTCaptureDeviceInput * captureDeviceInput;

    // One or the other of these may be used; generally
    // there's no need to use both?:
    QTCaptureDecompressedVideoOutput * decompressedVideoOutput;
    QTCaptureVideoPreviewOutput * videoPreviewOutput;

    CIFilter * colorFilter;
    CIFilter * gammaFilter;

    BOOL flipImageVertically;
    int minimumFrameRate;

    CVImageBufferRef lastCapturedImageBuffer;
    QTTime lastSampleTime; // set by QTCapture callback
    QTTime lastCaptureTime; // copied from above when frame is
    // obtained by dwyco (won't change til
    // next time getLastCapturedImageAsBitmap is called)
    NSObject * lastCapturedImageBufferMutex;
    int capWidth;
    int capHeight;
}
- (id) init;
- (id) initWithCaptureDeviceId:(NSString *) captureDeviceId;
- (id) initWithCaptureDevice:(QTCaptureDevice *) captureDevice;

- (void) changeCaptureDeviceById:(NSString *) newDeviceId;
- (void) changeCaptureDevice:(QTCaptureDevice *) newDevice;

- (float) gamma;
- (float) brightness;
- (float) contrast;
- (float) saturation;

/**
 * Set gamma value; range is 0.1 to 3.0, 1.0 normal.
 */
- (void) setGamma:(float) gamma;

/**
 * Set brightness value; range is -1.0 to 1.0, 0.0 normal.
 */
- (void) setBrightness:(float) brightness;

/**
 * Set contrast value; range is 0.25 to 4.0, 1.0 normal.
 */
- (void) setContrast:(float) contrast;

/**
 * Set saturation value; range is 0.0 to 2.0, 1.0 normal.
 */
- (void) setSaturation:(float) saturation;

/**
 * Set a flag indicating that image pixel rows need to be reversed vertically.
 */
- (void) setFlipImageVertically:(BOOL) flip;

//- (void) setMinimumFrameRate:(int) fps;

/**
 * Restore default camera settings (also restores them to preferences).
 */
- (void) restoreDefaultSettings;

/**
 * Save current camera settings to preferences.
 */
- (void) saveSettingsToPreferences;

/**
 * Load camera settings from preferences and apply.
 */
- (void) loadSettingsFromPreferences;

// MacDwycoVideoSource delegate protocol
- (BOOL)		hasCaptureData;
- (void)		startCapture;
- (void)		stopCapture;
- (NSData *)	getLatestCapturedRGBFrameBytes96x128;
- (NSData *)	getLatestCapturedRGBFrameBytes: (int *) width height:(int *) height;
- (CVImageBufferRef) getLatestCapturedRGBImageBuffer: (int *)width height:(int *)height;
- (QTTime)		lastCaptureTime;
- (void)		showSourceDialog;
- (void)		previewOn;
- (void)		previewOff;
- (BOOL)		flipImageVertically;
- (void)		setCaptureSize: (int) width height:(int) height;
- (int)			isCapturing;

- (QTCaptureSession *) captureSession;
- (QTCaptureDevice *) captureDevice;
- (QTCaptureDeviceInput *) captureDeviceInput;
- (QTCaptureVideoPreviewOutput *) videoPreviewOutput;
- (QTCaptureDecompressedVideoOutput *) decompressedVideoOutput;
- (QTTime) lastSampleTime;

/**
 * Get the image buffer of the last video frame, optionally setting it to nil
 * and optionally forcing thread safety.
 * @param setToNil Whether to set the field holding this object to nil
 *  set the field to nil; if TRUE, this object will no longer have a reference to the buffer
 *  hasCaptureData will return false until a new buffer is received.  It is the
 *  caller's responsibility in this case to release the buffer with CVBufferRelease().
 * @param synchronized Whether to use a @synchronized block to access and release the
 *  buffer; if releaseAndNil is FALSE this parameter is ignored.
 */
- (CVImageBufferRef) lastCapturedImageBuffer:(BOOL)setToNil synchronized:(BOOL) threadSafe;

/**
 * Get a bitmap representation of the last captured image buffer.
 * @note This call is thread-safe and on exit the lastCapturedImageBuffer will be nil
 *   (so that hasCaptureData will return FALSE until another frame is received).
 * @param width The desired bitmap width
 * @param height The desired bitmap height
 * @return NSBitmapImageRep The bitmap (as 24-bit RGB).
 */
- (NSBitmapImageRep *) getLastCapturedImageAsBitmap:(int) width height:(int) height;

/**
 * Convert a CoreVideo image buffer to an RGB bitmap, applying CI filters; used by getLastCapturedImageAsBitmap.
 * @param cvImage A core video image buffer.
 * @param targetWidth The desired bitmap width.
 * @param height The desired bitmap height.
 * @return NSBitmapImageRep A bitmap representation of the core video frame, as 24-bit RGB.
 */
- (NSBitmapImageRep *) coreVideoImageToBimap:(CVImageBufferRef) cvImage targetWidth:(int) width height:(int) height;

/**
 * Class utility method to obtain raw bitmap bytes from an NSBitmapImageRep.
 * @param bitmapRep An NSBitmapImageRep.
 * @return NSData containing the bitmap bytes, typically in packed 24-bit RGB (three consecutive bytes per pixel as r,g,b).
 */
+ (NSData *) bitmapDataFromBitmapImageRep:(NSBitmapImageRep *) bitmapRep;

/**
 * Class utility method to find a QTCaptureDevice by ID.
 * <p>This function always queries the OS for available devices, it does not rely on a
 * cached list.
 * @param deviceId The QTCaptureDevice id, as returned by its uniqueId method.
 * @return QTCaptureDevice The device with the matching id, or nil if not found.
 */
+ (QTCaptureDevice *) lookUpCaptureDeviceById:(NSString *) deviceId;

@end
