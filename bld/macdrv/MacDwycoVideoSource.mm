
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
//  MacDwycoVideoSource.mm
//  DwycoVideoTest
//
//  Created by Christopher Corbell on 8/26/09.
//

#import "MacDwycoVideoSource.h"
#import "MacDwycoCameraSettingsController.h"

/*
Developer notes:

 There is likely room for optimization here; this code should eventually be profiled
 and improved for resource usage and performance.

 Under Mac OS X 10.6 there are two new methods on QTCaptureVideoPreviewOutput:
   setMinimumVideoFrameInterval:
   setAutomaticallyDropsLateFrames:
 I do not yet have 10.6 so I can't see the proper signature of these, but we
 should add conditional use of these methods when running on 10.6 or later.
*/

@implementation MacDwycoVideoSource
- (id) init
{
    self = [super init];
    if(self != nil)
    {
        captureSession = nil;
        captureDevice = nil;
        captureDeviceInput = nil;
        decompressedVideoOutput = nil;
        videoPreviewOutput = nil;
        colorFilter = nil;
        gammaFilter = nil;
        lastCapturedImageBufferMutex = 0;
        flipImageVertically = 0;
        minimumFrameRate = 12;
        capWidth = 0;
        capHeight = 0;
    }
    return self;

}
- (id) initWithCaptureDeviceId:(NSString *) captureDeviceId
{
    QTCaptureDevice * foundDevice = [MacDwycoVideoSource lookUpCaptureDeviceById:captureDeviceId];
    if (!foundDevice)
    {
        NSLog(@"ERROR - cannot find video device with ID %@", captureDeviceId);
        [[super init] autorelease];
        return nil;
    }
    return [self initWithCaptureDevice:foundDevice];
}

- (id) initWithCaptureDevice:(QTCaptureDevice *) captureDeviceParam;
{
    self = [super init];
    if (nil != self)
    {
        //capWidth = 0;
        //capHeight = 0;
        captureDevice = captureDeviceParam;
        captureSession = [[QTCaptureSession alloc] init];

        NSError * error = nil;
        if (![captureDevice open:&error])
        {
            NSLog(@"Error: failed to open device: %@", error);
        }
        else
        {
            captureDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:captureDevice];
            if (![captureSession addInput:captureDeviceInput error:&error])
            {
                NSLog(@"Error adding video input to capture session: %@", error);
            }


            videoPreviewOutput = [[QTCaptureVideoPreviewOutput alloc] init];

            NSDictionary * pixelBufferAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                                    [NSNumber numberWithDouble:capWidth], (id)kCVPixelBufferWidthKey,
                                                    [NSNumber numberWithDouble:capHeight], (id)kCVPixelBufferHeightKey,
                                                    [NSNumber numberWithUnsignedInt:kCVPixelFormatType_24RGB/*k32ABGRPixelFormat*/],
                                                    (id)kCVPixelBufferPixelFormatTypeKey,
                                                    nil];

            [videoPreviewOutput setPixelBufferAttributes:pixelBufferAttributes];


            [videoPreviewOutput setDelegate:self];
            if (![captureSession addOutput:videoPreviewOutput error:&error])
            {
                NSLog(@"Error adding videoPreviewOutput to session: %@", error);
            }
        }
        lastCapturedImageBufferMutex = [[NSNumber numberWithInt:0] retain];
        @synchronized(lastCapturedImageBufferMutex)
        {
            lastCapturedImageBuffer = nil;
        }

        colorFilter = [[CIFilter filterWithName:@"CIColorControls"] retain];
        [colorFilter setDefaults];
        gammaFilter = [[CIFilter filterWithName:@"CIGammaAdjust"] retain];
        [gammaFilter setDefaults];
        flipImageVertically = YES;

        //minimumFrameRate = 30;

        [self loadSettingsFromPreferences];
    }
    return self;
}

- (void) setCaptureSize: (int) width height:(int) height
{
    capWidth = width;
    capHeight = height;
}

- (void) changeCaptureDeviceById:(NSString *) newDeviceId
{
    if (![newDeviceId isEqualToString:[captureDevice uniqueID]])
    {
        QTCaptureDevice * foundDevice = [MacDwycoVideoSource lookUpCaptureDeviceById:newDeviceId];
        if (!foundDevice)
            NSLog(@"ERROR - cannot find video device with ID %@", newDeviceId);
        else
            [self changeCaptureDevice:foundDevice];
    }
}

- (void) changeCaptureDevice:(QTCaptureDevice *) newDevice
{
    if (![[newDevice uniqueID] isEqualToString:[captureDevice uniqueID]])
    {
        if ([captureSession isRunning])
            [captureSession stopRunning];

        [captureSession removeInput:captureDeviceInput];
        [captureDevice close]; // We don't release because we didn't retain (just relied on session's retain)

        if (nil != captureDeviceInput)
        {
            [captureDeviceInput autorelease];
            captureDeviceInput = Nil;
        }

        NSError * error = Nil;
        captureDevice = newDevice;
        if (![captureDevice open:&error])
        {
            NSLog(@"Error opening video device: %@", error);
        }
        else
        {
            captureDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:captureDevice];
            if (![captureSession addInput:captureDeviceInput error:&error])
            {
                NSLog(@"Error opening video device capture: %@", error);
            }
        }
    }
}

- (float) gamma
{
    NSNumber * value = [gammaFilter valueForKey:@"inputPower"];
    return [value floatValue];
}

- (float) brightness
{
    NSNumber * value = [colorFilter valueForKey:@"inputBrightness"];
    return [value floatValue];
}

- (float) contrast
{
    NSNumber * value = [colorFilter valueForKey:@"inputContrast"];
    return [value floatValue];
}

- (float) saturation
{
    NSNumber * value = [colorFilter valueForKey:@"inputSaturation"];
    return [value floatValue];
}

- (BOOL) flipImageVertically
{
    return flipImageVertically;
}

- (void) setGamma:(float) gamma
{
    [gammaFilter setValue:[NSNumber numberWithFloat:gamma] forKey:@"inputPower"];
}

- (void) setBrightness:(float) brightness
{
    [colorFilter setValue:[NSNumber numberWithFloat:brightness] forKey:@"inputBrightness"];
}

- (void) setContrast:(float) contrast
{
    [colorFilter setValue:[NSNumber numberWithFloat:contrast] forKey:@"inputContrast"];
}

- (void) setSaturation:(float) saturation
{
    [colorFilter setValue:[NSNumber numberWithFloat:saturation] forKey:@"inputSaturation"];
}

- (void) setFlipImageVertically:(BOOL) flip
{
    flipImageVertically = flip;
}

/*
- (void) setMinimumFrameRate:(int) fps
{
	minimumFrameRate = fps;
	NSTimeInterval frameInterval = 0.0;
	if (fps != 0)
		frameInterval = 1.0 / (NSTimeInterval) fps;
	// Oops - this is in the documentation but it isn't supported until OS 10.6 & a newer QuickTime than I have
	//[decompressedVideoOutput setMinimumVideoFrameInterval:frameInterval];
}
*/

- (void) restoreDefaultSettings
{
    [gammaFilter setDefaults];
    [colorFilter setDefaults];
    flipImageVertically = YES;
}

- (void) saveSettingsToPreferences
{
    NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary * settings = [NSDictionary dictionaryWithObjectsAndKeys:
                               [gammaFilter valueForKey:@"inputPower"],
                               @"gamma.inputPower",
                               [colorFilter valueForKey:@"inputBrightness"],
                               @"color.inputBrightness",
                               [colorFilter valueForKey:@"inputContrast"],
                               @"color.inputContrast",
                               [colorFilter valueForKey:@"inputSaturation"],
                               @"color.inputSaturation",
                               [NSNumber numberWithBool:flipImageVertically],
                               @"flipImageVertically",
                               [NSNumber numberWithInt:minimumFrameRate],
                               @"minimumFrameRate",
                               nil];
    [defaults setObject:settings forKey:@"com.dwyco.video.camera-settings"];
}

- (void) loadSettingsFromPreferences
{
    NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary * settings = [defaults dictionaryForKey:@"com.dwyco.video.camera-settings"];
    if (nil != settings)
    {
        NSNumber * value = [settings valueForKey:@"gamma.inputPower"];
        if (nil != value)
            [gammaFilter setValue:value	forKey:@"inputPower"];

        value = [settings valueForKey:@"color.inputBrightness"];
        if (nil != value)
            [colorFilter setValue:value	forKey:@"inputBrightness"];

        value = [settings valueForKey:@"color.inputContrast"];
        if (nil != value)
            [colorFilter setValue:value	forKey:@"inputContrast"];

        value = [settings valueForKey:@"color.inputSaturation"];
        if (nil != value)
            [colorFilter setValue:value	forKey:@"inputSaturation"];

        value = [settings valueForKey:@"flipImageVertically"];
        if (nil != value)
            flipImageVertically = [value boolValue];

        //value = [settings valueForKey:@"minimumFrameRate"];
        //if (nil != value)
        //	[self setMinimumFrameRate:[value intValue]];
    }
}



- (void) dealloc
{
    if ([captureSession isRunning])
        [captureSession stopRunning];
    [captureSession removeInput:captureDeviceInput];

    [captureSession removeOutput:videoPreviewOutput];
    //[captureSession removeOutput:decompressedVideoOutput];

    if ([captureDevice isOpen])
        [captureDevice close];
    [captureDeviceInput release];

    [videoPreviewOutput release];
    //[decompressedVideoOutput release];

    [captureSession release];
    CVImageBufferRef cvBufToRelease = nil;
    @synchronized(lastCapturedImageBufferMutex)
    {
        if (lastCapturedImageBuffer != nil)
        {
            cvBufToRelease = lastCapturedImageBuffer;
            lastCapturedImageBuffer = nil;
        }

        if (cvBufToRelease != nil)
        {
            CVBufferRelease(cvBufToRelease);
        }
    }
    [lastCapturedImageBufferMutex release];
    [super dealloc];
}

- (BOOL) hasCaptureData
{
    int ret;
    @synchronized(lastCapturedImageBufferMutex)
    {
        ret = (lastCapturedImageBuffer != nil && capWidth != 0 && capHeight != 0);
    }
    return ret;
}

- (void) startCapture
{
    [captureSession startRunning];
}

- (void) stopCapture
{
    [captureSession stopRunning];
}

- (int) isCapturing
{
    return [captureSession isRunning];
}

- (NSData *) getLatestCapturedRGBFrameBytes96x128
{
    NSBitmapImageRep * bitmapImageRep = [self getLastCapturedImageAsBitmap:128 height:96];
    return [MacDwycoVideoSource bitmapDataFromBitmapImageRep:bitmapImageRep];
}

- (NSData *) getLatestCapturedRGBFrameBytes: (int *)width height:(int *)height
{
    NSBitmapImageRep * bitmapImageRep = [self getLastCapturedImageAsBitmap:capWidth height:capHeight];
    *width = capWidth;
    *height = capHeight;
    return [MacDwycoVideoSource bitmapDataFromBitmapImageRep:bitmapImageRep];
}

- (CVImageBufferRef) getLatestCapturedRGBImageBuffer: (int *)width height:(int *)height
{
    // note: ImageBuffer is guaranteed to be a PixelBuffer according to docs, as long as
    // you explicitly set the "pixelBufferAttributes"
    CVPixelBufferRef imageBuffer = (CVPixelBufferRef)[self lastCapturedImageBuffer:TRUE synchronized:TRUE];

    *width = CVPixelBufferGetWidth(imageBuffer);
    *height =  CVPixelBufferGetHeight(imageBuffer);
    return imageBuffer;
}

- (void) showSourceDialog
{
    [MacDwycoCameraSettingsController showPanelForSource:self];
}

- (void) previewOn
{
    // TODO: broadcast a notification that preview should be turned on, OR
    // make QTCaptureView explicitly register with this object
    //NSLog(@"%s", __FUNCTION__);
}

- (void) previewOff
{
    // TODO: broadcast a notification that preview should be turned off, OR
    // make QTCaptureView explicitly register with this object
    //NSLog(@"%s", __FUNCTION__);
}


- (QTCaptureSession *) captureSession
{
    return captureSession;
}

- (QTCaptureDevice *) captureDevice
{
    return captureDevice;
}

- (QTCaptureDeviceInput *) captureDeviceInput
{
    return captureDeviceInput;
}

- (QTCaptureVideoPreviewOutput *) videoPreviewOutput
{
    return videoPreviewOutput;
}

- (QTCaptureDecompressedVideoOutput *) decompressedVideoOutput
{
    return decompressedVideoOutput;
}

- (QTTime) lastSampleTime
{
    return lastSampleTime;
}

- (QTTime) lastCaptureTime
{
    return lastCaptureTime;
}

- (CVImageBufferRef) lastCapturedImageBuffer:(BOOL)setToNil synchronized:(BOOL) threadSafe;
{
    if (setToNil)
    {
        CVImageBufferRef imageBuffer = nil;
        if (threadSafe)
        {
            @synchronized(lastCapturedImageBufferMutex) {
                imageBuffer = lastCapturedImageBuffer;
                lastCapturedImageBuffer = nil;
                lastCaptureTime = lastSampleTime;
            }
        }
        else
        {
            imageBuffer = lastCapturedImageBuffer;
            lastCapturedImageBuffer = nil;
        }
        return imageBuffer;
    }
    return lastCapturedImageBuffer;
}

- (NSBitmapImageRep *) getLastCapturedImageAsBitmap:(int) width height:(int) height
{
    CVImageBufferRef imageBuffer = [self lastCapturedImageBuffer:TRUE synchronized:TRUE];

    NSBitmapImageRep * bitmapImageRep = [self coreVideoImageToBimap:imageBuffer targetWidth:width height:height];
    // To investigate: this gives me a 24-bit RGB bitmap with 3 bytes per pixel; is there any chance
    // a different format could result based on input video source?

    // release the buffer which was retained in the qtkit callback
    CVBufferRelease(imageBuffer);

    return bitmapImageRep;
}

- (NSBitmapImageRep *) coreVideoImageToBimap:(CVImageBufferRef) cvImage targetWidth:(int) width height:(int) height
{
    if (nil != cvImage)
    {
        CIImage * inputImage = [CIImage imageWithCVImageBuffer:cvImage];

        [colorFilter setValue:inputImage forKey:@"inputImage"];
        CIImage * colorAdjustedImage = [colorFilter valueForKey:@"outputImage"];

        [gammaFilter setValue:colorAdjustedImage forKey:@"inputImage"];
        CIImage * gammaAdjustedImage = [gammaFilter valueForKey:@"outputImage"];

        NSCIImageRep * imageRep = [NSCIImageRep imageRepWithCIImage:gammaAdjustedImage];

        //NSCIImageRep * imageRep = [NSCIImageRep imageRepWithCIImage: [CIImage imageWithCVImageBuffer:cvImage]];
        NSImage *image = [[[NSImage alloc] initWithSize:[imageRep size]] autorelease];
        [image addRepresentation:imageRep];

        NSSize imageSize = [image size];
        if (imageSize.height != height || imageSize.width != width)
        {
            [image setScalesWhenResized:YES];
            [image setSize:NSMakeSize(width, height)];
        }

        NSData * tiffData = [image TIFFRepresentationUsingCompression:NSTIFFCompressionNone factor:0];
        NSBitmapImageRep * bitmap = [[[NSBitmapImageRep alloc] initWithData:tiffData] autorelease];

        [colorFilter setValue:nil forKey:@"inputImage"];
        [gammaFilter setValue:nil forKey:@"inputImage"];

        return bitmap;
    }
    return nil;
}

+ (NSData *) bitmapDataFromBitmapImageRep:(NSBitmapImageRep *) bitmapRep
{
    return [NSData dataWithBytes:[bitmapRep bitmapData] length:[bitmapRep bytesPerRow] * [bitmapRep pixelsHigh]];
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput
    didOutputVideoFrame:(CVImageBufferRef)videoFrame
    withSampleBuffer:(QTSampleBuffer *)sampleBuffer
    fromConnection:(QTCaptureConnection *)connection

{
    //NSLog(@"got a QTCaptureDecompressedVideoOutput captureOutput callback!");

    // NOTE: sample time as QTTime is available as part of QTSampleBuffer, we'll need to set this as well

    // Store the latest frame
    // This must be done in a @synchronized block because this delegate method is not called on the main thread
    CVImageBufferRef imageBufferToRelease = nil;
    @synchronized (lastCapturedImageBufferMutex) {
        imageBufferToRelease = lastCapturedImageBuffer;
        lastCapturedImageBuffer = videoFrame;
        CVBufferRetain(lastCapturedImageBuffer);
        lastSampleTime = [sampleBuffer presentationTime];


        if (nil != imageBufferToRelease)
        {
            CVBufferRelease(imageBufferToRelease);
        }
    }
}

+ (QTCaptureDevice *) lookUpCaptureDeviceById:(NSString *) deviceId
{
    QTCaptureDevice * foundDevice = nil;
    NSArray * devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];

    for (int index = 0; index < [devices count]; index++)
    {
        QTCaptureDevice * device = [devices objectAtIndex:index];
        if ([deviceId isEqualToString:[device uniqueID]])
        {
            foundDevice = device;
            break;
        }
    }

    if(nil == foundDevice)
    {
        // Try multiplexed devices
        devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed];
        for (int index = 0; index < [devices count]; index++)
        {
            QTCaptureDevice * device = [devices objectAtIndex:index];
            if ([deviceId isEqualToString:[device uniqueID]])
            {
                foundDevice = device;
                break;
            }
        }
    }

    return foundDevice;
}

@end
