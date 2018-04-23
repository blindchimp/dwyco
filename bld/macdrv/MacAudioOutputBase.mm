
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import "MacAudioOutputBase.h"


@implementation MacAudioOutputBase

static const int kNumBuffers = 4;
static const int kMaxFramesPerBuffer = 3528;

static void MacAudioOutputBaseCallback(void * inUserData,
                                       AudioQueueRef inAQ,
                                       AudioQueueBufferRef inCompleteAQBuffer)
{
    MacAudioOutputBase * mdAudioOut = (MacAudioOutputBase *) inUserData;
    [mdAudioOut outputCallback:inAQ buffer:inCompleteAQBuffer];
}

- (id) init
{
    self = [super init];
    if (self)
    {
        audioStreamDesc.mFormatID = kAudioFormatLinearPCM;
        audioStreamDesc.mSampleRate = 44100.0;
        audioStreamDesc.mChannelsPerFrame = 1;
        audioStreamDesc.mBitsPerChannel = 16;
        audioStreamDesc.mBytesPerPacket = sizeof(SInt16);
        audioStreamDesc.mBytesPerFrame = sizeof(SInt16);
        audioStreamDesc.mFramesPerPacket = 1;
        audioStreamDesc.mFormatFlags =	kLinearPCMFormatFlagIsSignedInteger;// | kLinearPCMFormatFlagIsPacked;

        audioQueue = NULL;

        OSStatus err = AudioQueueNewOutput(&audioStreamDesc,
                                           MacAudioOutputBaseCallback,
                                           self,
                                           NULL, //CFRunLoopGetCurrent(),
                                           NULL,
                                           0,
                                           &audioQueue);
        if (err == noErr)
        {
            for (int n = 0; n < kNumBuffers; n++)
            {
                AudioQueueBufferRef buffer;
                err = AudioQueueAllocateBuffer(audioQueue , sizeof(short) * kMaxFramesPerBuffer, &buffer);
                if (err != noErr)
                {
                    NSLog(@"AudioQueueAllocateBuffer returned %d", (int)err);
                }
                else
                {

                    // Prime the queue with first few buffers worth of data - just set 1 silent frame in each buffer
                    buffer->mAudioDataByteSize = kMaxFramesPerBuffer * sizeof(short);
                    memset(buffer->mAudioData, 0, buffer->mAudioDataByteSize);
                    err = AudioQueueEnqueueBuffer(audioQueue, buffer, 0, NULL);
                    if (err != noErr)
                    {
                        NSLog(@"AudioQueueEnqueueBuffer returned %d", (int)err);
                    }

                }
            }

            if (err == noErr)
            {
                err = AudioQueueSetParameter(audioQueue, kAudioQueueParam_Volume, 1.0f);
                if (err != noErr)
                    NSLog(@"AudioQueueSetParameter returned %d", (int)err);
            }

        }
        else
        {
            NSLog(@"AudioQueueNewOutput returned %d", (int)err);
        }
    }
    return self;
}

- (void) dealloc
{
    if (nil != audioQueue && queueRunning == TRUE)
    {
        [self stopQueue];
        OSStatus status = AudioQueueDispose(audioQueue, YES);
        if (status != 0)
        {
            NSLog(@"AudioQueueDispose error: %d", (int)status);
        }
    }
    [super dealloc];
}

- (void) setDelegate:(id) del
{
    if (del != nil)
    {
        if (![del respondsToSelector:@selector(outputCallback:buffer:)])
        {
            NSLog(@"ERROR: %s - delegate does not respond to required selector.", __FUNCTION__);
        }
    }
    self->delegate = del;
}

- (OSStatus) startQueue
{
    //NSLog(@"%s", __FUNCTION__);
    OSStatus err = AudioQueueStart(audioQueue, NULL);
    if (err != noErr)
    {
        NSLog(@"ERROR - AudioQueueStart returned %d", (int)err);
    }
    else
    {
        queueRunning = TRUE;
    }
    return err;
}

- (OSStatus) stopQueue
{
    //NSLog(@"%s", __FUNCTION__);
    queueRunning = FALSE;
    return AudioQueueStop(audioQueue, TRUE);
}

- (OSStatus) pauseQueue
{
    //NSLog(@"%s", __FUNCTION__);
    queueRunning = FALSE;
    return AudioQueuePause(audioQueue);
}

- (OSStatus) flushQueue
{
    //NSLog(@"%s", __FUNCTION__);
    return AudioQueueFlush(audioQueue);
}

- (BOOL) isQueueRunning
{
    return queueRunning && (audioQueue != nil);
}

- (AudioQueueRef) audioQueue
{
    //NSLog(@"%s", __FUNCTION__);
    return audioQueue;
}

- (void) outputCallback:(AudioQueueRef)inAQ buffer:(AudioQueueBufferRef) inCompleteAQBuffer
{
    //NSLog(@"%s", __FUNCTION__);
    if (delegate != nil)
    {
        [delegate outputCallback:inAQ buffer:inCompleteAQBuffer];
    }
    else
    {
        NSLog(@"%s - no delegate...", __FUNCTION__);
    }
}

@end
