
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import "MacDwyco.h"
#import "MacDwycoVideoSource.h"
#import "MacDwycoAudioSource.h"

//#import "DwycoServer.h"

#import <QTKit/QTKit.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "ppm.h"
#include "vidaq.h"
//#include "vidskel.h"
//#include "dlli.h"

// Callback declarations:
void DWYCOEXPORT	mac_vg_set_appdata(void *u1);
void DWYCOEXPORT	mac_vgnew(void *aqext);
void DWYCOEXPORT	mac_vgdel(void *aqext);
int DWYCOEXPORT		mac_vginit(void *aqext, int frame_rate);
int DWYCOEXPORT		mac_vghas_data(void *aqext);
void DWYCOEXPORT	mac_vgneed(void *aqext);

void DWYCOEXPORT	mac_vgpass(void *aqext);

void DWYCOEXPORT	mac_vgstop(void *aqext);
void * DWYCOEXPORT	mac_vgget_data(void *aqext,
                                   int *c_out,
                                   int *r_out,
                                   int *bytes_out,
                                   int *fmt_out,
                                   unsigned long *captime_out);

void DWYCOEXPORT	mac_vgfree_data(void *data);

char ** DWYCOEXPORT	mac_vgget_video_devices();
void DWYCOEXPORT	mac_vgfree_video_devices(char **);
void DWYCOEXPORT	mac_vgset_video_device(int idx);
void DWYCOEXPORT	mac_vgstop_video_device();
void DWYCOEXPORT	mac_vgshow_source_dialog();
// these are needed so we can show video preview
// while the source dialog is up and adjusting
void DWYCOEXPORT	mac_vgpreview_on(void *display_window);
void DWYCOEXPORT	mac_vgpreview_off();

//void DWYCOCALLCONV	blit_ppm(int ui_id, void *vimg, int cols, int rows, int depth);
//void DWYCOCALLCONV	init_video_display(int chan_id, int ui_id);

void DWYCOCALLCONV	mac_audio_new(void *, int buflen, int nbufs);
void DWYCOCALLCONV	mac_audio_delete(void *);
int DWYCOCALLCONV	mac_audio_init(void *);
int DWYCOCALLCONV	mac_audio_has_data(void *);
void DWYCOCALLCONV	mac_audio_need(void *);
void DWYCOCALLCONV	mac_audio_pass(void *);
void DWYCOCALLCONV	mac_audio_stop(void *);
void DWYCOCALLCONV	mac_audio_on(void *);
void DWYCOCALLCONV	mac_audio_off(void *);
void DWYCOCALLCONV	mac_audio_reset(void *);
int DWYCOCALLCONV	mac_audio_status(void *);
void * DWYCOCALLCONV mac_audio_get_data(void *e, int *len, int *time);


static MacDwyco * singleton_instance = Nil;

@implementation MacDwyco

+ (MacDwyco *) singleton
{
    if (singleton_instance == Nil)
    {
        singleton_instance = [[MacDwyco alloc] init];
    }
    return singleton_instance;
}

- (id) init
{
    self = [super init];
    if (self)
    {
        //blitCallbackDelegates = [[NSMutableDictionary alloc] init];
        videoInputDevices = nil;
        currentVideoInputDeviceIndex = 0;
        videoSourceDelegate = nil;
        audioSource = nil;
    }
    return self;
}

- (void) setCurrentVideoInputDeviceIndex:(int) index
{
    int numDevices = 0;
    if (videoInputDevices != nil)
    {
        numDevices = [videoInputDevices count];
    }
    if (index <= numDevices)
    {
        currentVideoInputDeviceIndex = index;
    }
}

- (int) getCurrentVideoInputDeviceIndex
{
    return currentVideoInputDeviceIndex;
}

- (NSArray *) getVideoInputDevices:(BOOL) refreshList
{
    if (refreshList)
    {
        NSArray * videoDevices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];
        NSArray * muxedDevices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed];
        if (videoInputDevices != nil)
        {
            [videoInputDevices autorelease];
        }
        videoInputDevices = [[videoDevices arrayByAddingObjectsFromArray:muxedDevices] retain];
    }
    return videoInputDevices;
}

- (QTCaptureDevice *) getCurrentVideoInputDevice
{
    return (QTCaptureDevice *) [videoInputDevices objectAtIndex:currentVideoInputDeviceIndex];
}


- (void) setVideoSourceDelegate:(NSObject<MacDwycoVideoSourceDelegate> *) delegate
{
    if (videoSourceDelegate != nil)
    {
        if(delegate != 0)
            ::abort();
        // handling device objects explicitly so we know the device is
        // released at fixed time
        //[videoSourceDelegate autorelease];
    }
    videoSourceDelegate = delegate;
}

/**
 * Get the video source delegate.
 */
- (NSObject<MacDwycoVideoSourceDelegate> *) getVideoSourceDelegate
{
    return (NSObject<MacDwycoVideoSourceDelegate> *) videoSourceDelegate;
}

- (void) setAudioSource:(MacDwycoAudioSource *) source
{
    if (nil != audioSource)
    {
        if ([audioSource isQueueRunning])
            [audioSource off];
        [audioSource release];
        audioSource = nil;
    }
    audioSource = source;
    [audioSource retain];
}

- (MacDwycoAudioSource *) getAudioSource
{
    return audioSource;
}

@end

#define NVIDSIZES 4
static int Sizes[NVIDSIZES][2] = {
    {320, 240},
    {160, 120},
    {640, 480},
    {1280, 720}
};


char **
DWYCOEXPORT
mac_vgget_video_devices()
{
    //fprintf(stderr, "vgget_video_devices\n");
    // for now, assume we can do 160x120 and 320x240 on
    // all devices.

    NSArray * devices = [[MacDwyco singleton] getVideoInputDevices:TRUE];
    NSLog(@"got %lu video or muxed devices:\n %@", (unsigned long)[devices count], devices);

    int numDevices = [devices count];
    // we'll pull the same trick here we did with the windows dx9 stuff,
    // which is to create "devices" with explicit sizes attached to them
    // and then select the size internally based on the index
    char ** results = new char * [NVIDSIZES * numDevices + 1];

    for (int index = 0; index < numDevices; index++)
    {
        NSString *a = [NSString stringWithFormat:@"Large (~320x240) Camera #%d",  index + 1];
        results[index * NVIDSIZES] = strdup([a UTF8String]);

        a = [NSString stringWithFormat:@"Small (~160x120) Camera #%d",  index + 1];
        results[index * NVIDSIZES + 1] = strdup([a UTF8String]);

        a = [NSString stringWithFormat:@"Huge (~640x480) Camera #%d",  index + 1];
        results[index * NVIDSIZES + 2] = strdup([a UTF8String]);

        a = [NSString stringWithFormat:@"HD 720 (~1280x720) Camera #%d",  index + 1];
        results[index * NVIDSIZES + 3] = strdup([a UTF8String]);

    }
    results[numDevices * NVIDSIZES] = 0;
    return results;
}

void
DWYCOEXPORT
mac_vgfree_video_devices(char **d)
{
    //fprintf(stderr, "vgfree_video_devices\n");
    char **tmp = d;
    while(*d)
    {
        free(*d);
        ++d;
    }
    delete [] tmp;
}

// if the video cap device is in use, we
// turn off the old one, select the new one, and
// restart capturing.
void
DWYCOEXPORT
mac_vgset_video_device(int idx)
{
    mac_vgstop_video_device();
    //[[[MacDwyco singleton] getVideoSourceDelegate] dealloc];
    //[[MacDwyco singleton] setVideoSourceDelegate: 0];
    MacDwycoVideoSource *d = [[MacDwycoVideoSource alloc] init];
    [d setCaptureSize:Sizes[idx % NVIDSIZES][0] height:Sizes[idx % NVIDSIZES][1]];

    [[MacDwyco singleton] setVideoSourceDelegate: d];
    // client is using expanded indexs, we divide by 2
    // to get the actual device index
    [[MacDwyco singleton] setCurrentVideoInputDeviceIndex:(idx / NVIDSIZES)];
    mac_vginit(0, 0);
    // based on the idx, tell the capture object the size to use


}

// this should shutdown and release the device.
void
DWYCOEXPORT
mac_vgstop_video_device()
{
    mac_vgstop(0);
    mac_vgpass(0);
    [[[MacDwyco singleton] getVideoSourceDelegate] stopCapture];
    [[[MacDwyco singleton] getVideoSourceDelegate] dealloc];
    [[MacDwyco singleton] setVideoSourceDelegate: 0];
}

void
DWYCOEXPORT
mac_vgshow_source_dialog()
{
    [[[MacDwyco singleton] getVideoSourceDelegate] showSourceDialog];
}


void DWYCOEXPORT
mac_vg_set_appdata(void * u1)
{
    //fprintf(stderr, "vg_set_appdata\n");
}

void
DWYCOEXPORT
mac_vgnew(void *aqext)
{
    // new video capture object created
    //fprintf(stderr, "vgnew\n");
    if([[MacDwyco singleton] getVideoSourceDelegate] == 0)
    {
        MacDwycoVideoSource *d = [[MacDwycoVideoSource alloc] init];
        [[MacDwyco singleton] setVideoSourceDelegate: d];
    }
}

void
DWYCOEXPORT
mac_vgdel(void *aqext)
{
    //fprintf(stderr, "vgdel\n");
    mac_vgstop(0);
    mac_vgpass(0);
    // ok, we don't necessarily want to stop capturing in this case
#if 0
    [[[MacDwyco singleton] getVideoSourceDelegate] dealloc];
    [[MacDwyco singleton] setVideoSourceDelegate: 0];
#endif
}

int
DWYCOEXPORT
mac_vginit(void *aqext, int frame_rate)
{
    if([[MacDwyco singleton] getVideoSourceDelegate] == 0)
        return 0;

    if([[[MacDwyco singleton] getVideoSourceDelegate] isCapturing])
        return 1;

    //fprintf(stderr, "vginit\n");
    // TODO: set desired frame rate on video source delegate
    // bad: assumes the list of input devices hasn't changed. ok for now tho.
    NSArray * devices = [[MacDwyco singleton] getVideoInputDevices:TRUE];
    int idx = [[MacDwyco singleton] getCurrentVideoInputDeviceIndex];

    // i guess it is complaining because i'm implicitly "casting" the delegate to the specific subclass
    MacDwycoVideoSource *videoSource = (MacDwycoVideoSource *)[[MacDwyco singleton] getVideoSourceDelegate];
    [videoSource initWithCaptureDevice:[devices objectAtIndex:idx]];
    [videoSource startCapture];
    return 1;
}

int
DWYCOEXPORT
mac_vghas_data(void *aqext)
{
    int result = (int) [[[MacDwyco singleton] getVideoSourceDelegate] hasCaptureData];
    //fprintf(stderr, "vghas_data - returning %d\n", result);
    return result;
}

void
DWYCOEXPORT
mac_vgneed(void *aqext)
{
    // Not needed on Mac, but must be compiled here for current
    // binary library linkage (cdc32.a, shmcap.a)
}

void
DWYCOEXPORT
mac_vgpass(void *aqext)
{
    // Not needed on Mac, but must be compiled here for current
    // binary library linkage (cdc32.a, shmcap.a)
}

void
DWYCOEXPORT
mac_vgstop(void *aqext)
{
    [[[MacDwyco singleton] getVideoSourceDelegate] stopCapture];
}

void *
DWYCOEXPORT
mac_vgget_data(
    void *aqext,
    int *c_out, int *r_out,
    int *bytes_out, int *fmt_out, unsigned long *captime_out)
{
// NOTE NOTE NOTE!
//
// 128x96 is a fixed size needed by the theora codec as it is
// implemented in the current DLL. DO NOT CHANGE THE SIZE or the
// theora codec will choke. if this is a problem for testing, I can
// revert back to the previous codec, which is a bit more flexible in
// this area.
// here are the other sizes the theora codec understands right now:
//
    /*
        128, 96,
        176, 144,
        352, 288,
        704, 576,
        80, 64,
        160, 128,
        320, 240,
        640, 480
    */

    NSObject<MacDwycoVideoSourceDelegate> * source = [[MacDwyco singleton] getVideoSourceDelegate];

    //NSData * frameData = [source getLatestCapturedRGBFrameBytes96x128];
    int capht;
    int capwid;
    CVImageBufferRef frameData = [source getLatestCapturedRGBImageBuffer: &capwid height:&capht ];
    QTTime qtTime = [source lastCaptureTime];

    const int rowLength = CVPixelBufferGetBytesPerRow(frameData);
    int len = capwid * capht * 3;
    // dll is expecting packed data
    unsigned char * buf = (unsigned char *) malloc(len);
    if(CVPixelBufferLockBaseAddress(frameData, 0) != kCVReturnSuccess)
    {
        // bail out, returning garbage
        CVPixelBufferRelease(frameData);
        memset(buf, 0x55, len);
        *c_out = capwid;
        *r_out = capht;
        *fmt_out = (AQ_COLOR|AQ_RGB24);
        *bytes_out = len;
        return buf;
    }

    unsigned char * srcPtr = (unsigned char *) CVPixelBufferGetBaseAddress(frameData);
    unsigned char * destPtr = buf;

    if ([source flipImageVertically])
    {
        for (int n = capht - 1; n >= 0; n--)
        {
            unsigned char * rowToCopy = srcPtr + (n * rowLength);
            memcpy(destPtr, rowToCopy, 3 * capwid);
            destPtr += 3 * capwid;
        }
    }
    else
    {
        for (int n = 0; n < capht; n++)
        {
            unsigned char * rowToCopy = srcPtr + (n * rowLength);
            memcpy(destPtr, rowToCopy, 3 * capwid);
            destPtr += 3 * capwid;
        }
    }

    *c_out = capwid;
    *r_out = capht;
    *fmt_out = (AQ_COLOR|AQ_RGB24);
    *bytes_out = len;
    CVPixelBufferUnlockBaseAddress(frameData, 0);
    CVPixelBufferRelease(frameData);


    if (qtTime.timeScale != 0)
        *captime_out = qtTime.timeValue * 1000L / qtTime.timeScale;
    else
        *captime_out = qtTime.timeValue; // not likely to be milliseconds but better than dividing by zero!

    return buf;
#if 0
    NSData * frameData = [source getLatestCapturedRGBFrameBytes: &capwid height:&capht ];
    QTTime qtTime = [source lastCaptureTime];

    const int rowLength = capwid * 3;

    unsigned char * buf = (unsigned char *) malloc([frameData length]);

    unsigned char * srcPtr = (unsigned char *) [frameData bytes];
    unsigned char * destPtr = buf;

    if ([source flipImageVertically])
    {
        for (int n = capht - 1; n >= 0; n--)
        {
            unsigned char * rowToCopy = srcPtr + (n * rowLength);
            memcpy(destPtr, rowToCopy, rowLength);
            destPtr += rowLength;
        }
    }
    else
    {
        for (int n = 0; n < capht; n++)
        {
            unsigned char * rowToCopy = srcPtr + (n * rowLength);
            memcpy(destPtr, rowToCopy, rowLength);
            destPtr += rowLength;
        }
    }

    *c_out = capwid;
    *r_out = capht;
    *fmt_out = (AQ_COLOR|AQ_RGB24);
    *bytes_out = [frameData length];

    if (qtTime.timeScale != 0)
        *captime_out = qtTime.timeValue * 1000L / qtTime.timeScale;
    else
        *captime_out = qtTime.timeValue; // not likely to be milliseconds but better than dividing by zero!

    return buf;
#endif
}

void
DWYCOEXPORT
mac_vgfree_data(void * data)
{
    //fprintf(stderr, "vgfree_data\n");
    free(data);
}

void
DWYCOEXPORT
mac_vgpreview_on(void * display_window)
{
    [[[MacDwyco singleton] getVideoSourceDelegate] previewOn];
}

void
DWYCOEXPORT
mac_vgpreview_off()
{
    [[[MacDwyco singleton] getVideoSourceDelegate] previewOff];
}

#if 0
void
DWYCOCALLCONV
init_video_display(int chan_id, int ui_id)
{
    // associate chan_id with a ui-element (ui_id)
    NSLog(@"%s", __FUNCTION__);
    //fprintf(stderr, "associate chan %d with ui element %d\n", chan_id, ui_id);
    // TODO?
}
#endif

void DWYCOCALLCONV mac_audio_new(void *, int buflen, int nbufs)
{
    // If MacDwyco doesn't already have an audio source,
    // create a default one.
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * audioSource = (MacDwycoAudioSource *) [[MacDwyco singleton] getAudioSource];
    if (nil == audioSource)
    {
        audioSource = [[[MacDwycoAudioSource alloc] init] autorelease];
        [[MacDwyco singleton] setAudioSource:audioSource];
    }
    if ([audioSource packetSize] != buflen)
        [audioSource setPacketSize:buflen]; // or should it be buflen * numbufs?

    //NSLog(@"%s, buflen %d, nbufs %d", __FUNCTION__, buflen, nbufs);
}

void DWYCOCALLCONV mac_audio_delete(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * audioSource = (MacDwycoAudioSource *) [[MacDwyco singleton] getAudioSource];
    if (nil != audioSource)
    {
        [[MacDwyco singleton] setAudioSource:nil];
        if ([audioSource isQueueRunning])
            [audioSource off];
    }
}

int DWYCOCALLCONV mac_audio_init(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    return 1;
}

int DWYCOCALLCONV mac_audio_has_data(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    return [[[MacDwyco singleton] getAudioSource] hasCaptureData];
}

void DWYCOCALLCONV mac_audio_need(void *)
{
    //NSLog(@"%s", __FUNCTION__);
}

void DWYCOCALLCONV mac_audio_pass(void *)
{
    //NSLog(@"%s", __FUNCTION__);
}

void DWYCOCALLCONV mac_audio_stop(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * source = [[MacDwyco singleton] getAudioSource];
    if (nil != source && [source isQueueRunning])
        [source off];
}

void DWYCOCALLCONV mac_audio_on(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * source = [[MacDwyco singleton] getAudioSource];
    if(nil != source)
        [source on];
    //[[[MacDwyco singleton] getAudioSource] on];
}

void DWYCOCALLCONV mac_audio_off(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * source = [[MacDwyco singleton] getAudioSource];
    if (nil != source && [source isQueueRunning])
        [source off];
}

void DWYCOCALLCONV mac_audio_reset(void *)
{
    // Note; this does -not- call the source -reset method which
    // would basically make the source unusable (require complete
    // re-initialization)
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * source = [[MacDwyco singleton] getAudioSource];
    if (nil != source)
    {
        [source clearDataPackets];
    }
}

int DWYCOCALLCONV mac_audio_status(void *)
{
    //NSLog(@"%s", __FUNCTION__);
    return 1;
}

void * DWYCOCALLCONV mac_audio_get_data(void *e, int *len, int *time)
{
    //NSLog(@"%s", __FUNCTION__);

    MacAudioPacket packet = [[[MacDwyco singleton] getAudioSource] popLatestAudioPacket];
    if (packet.data != NULL)
    {
        *len = packet.dataSize;
        *time = (int) (packet.sampleTime);
        //NSLog(@"returning packet of data, length %d, time %d", *len, *time);
    }
    else
    {
        //NSLog(@"Did not get audio data this time...");
        *len = 0;
    }
    return packet.data; // dll will free this data
}

