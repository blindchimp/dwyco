
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import "MacDwycoAudioOutput.h"

#include <vector>
#include <deque>

#include "skelaudout.h"
//#include "dlli.h"
#include "audmix.h"
#include "audth.h"
#undef min


static const int kMaxFramesPerBuffer = 3528;
static MacDwycoAudioOutput * sharedAudioOutput = nil;

@implementation MacDwycoAudioOutput



+ (MacDwycoAudioOutput *) getSharedOutput
{
    if (sharedAudioOutput == nil)
    {
        sharedAudioOutput = [[MacDwycoAudioOutput alloc] init];
    }
    return sharedAudioOutput;
}

+ (void) releaseSharedOutput
{
    if (sharedAudioOutput != nil)
    {
        @synchronized(sharedAudioOutput)
        {
            [sharedAudioOutput release];
            sharedAudioOutput = nil;
        }
    }
}


- (id) init
{
    self = [super init];
    if (self)
    {
        audioDataQueue = new std::deque< std::vector<short> >;
        audioDataQueueMutex = [[NSNumber numberWithInt:1] retain];
    }
    return self;
}

- (void) dealloc
{
    [self resetDwycoAudioData];
    [super dealloc]; // stops and disposes of audioQueue

    std::deque< std::vector<short> > * temp = audioDataQueue;
    NSObject * tempMutex = audioDataQueueMutex;

    @synchronized(audioDataQueueMutex)
    {
        audioDataQueue = NULL;
        audioDataQueueMutex = nil;
    }
    delete temp;
    [tempMutex autorelease];
}

- (void) setDelegate:(id) del
{
    NSLog(@"ERROR: MacDwycoAudioOutput does not support the use of a delegate; ignoring %s invocation", __FUNCTION__);
    self->delegate = nil;
}

#if 0
+ (void) initializeDwycoCallbacks
{
    dwyco_set_external_audio_output_callbacks(
        skel_audout_new,
        skel_audout_delete,
        skel_audout_init,
        skel_audout_device_output,
        skel_audout_device_done,
        skel_audout_device_stop,
        skel_audout_device_reset,
        skel_audout_device_status,
        skel_audout_device_close,
        skel_audout_device_buffer_time,
        skel_audout_device_play_silence,
        skel_audout_device_bufs_playing
    );
}
#endif

- (void) pushSilence:(int) numSamples
{
    std::vector<short> vSamples;
    for (int n = 0; n < numSamples; n++)
        vSamples.push_back(0);
    [self pushDwycoReceivedAudioData:(char *) &vSamples[0] length:numSamples * sizeof(short)];
}

- (void) pushDwycoReceivedAudioData:(char *) data length:(int) length
{
    if (length %2 != 0)
    {
        NSLog(@"WARNING! expected audio to be even in length (2-byte integer frames), but it's %d;\nmay drop frame (if this is common, change implementation to handle it).", length);
        length --;
    }

    if (data == NULL || length <= 0 || audioDataQueueMutex == nil)
    {
        NSLog(@"Returning without doing anything; something is NULL");
        return;
    }

    short * shortData = (short *) data;

    int numFrames = length / 2;
    while (numFrames > 0)
    {
        std::vector<short> vAudioData;
        // Each vector we push back should contain at most kMaxFramesPerBuffer,
        // to simplify pulling each chunk into the AudioQueue.

        int thisBufferFrameCount = std::min(numFrames, kMaxFramesPerBuffer);
        for (int frame = 0; frame < thisBufferFrameCount; frame++)
        {
            vAudioData.push_back(shortData[frame]);
        }

        @synchronized(audioDataQueueMutex)
        {
            if (audioDataQueue != NULL)
                audioDataQueue->push_back(vAudioData);
            else {
                NSLog(@"WARNING - audioDataQueue is NULL, dropping %d frames of audio - if app is shutting down this may be expected.", numFrames);
            }
        }

        numFrames -= kMaxFramesPerBuffer;
        shortData = shortData + kMaxFramesPerBuffer;
    }
}

- (void) resetDwycoAudioData
{
    if ( audioDataQueueMutex == nil)
        return;

    @synchronized(audioDataQueueMutex)
    {
        if (audioDataQueue != NULL)
            audioDataQueue->clear();
    }
}

- (void) outputCallback:(AudioQueueRef)inAQ buffer:(AudioQueueBufferRef) inCompleteAQBuffer
{
    //NSLog(@"%s", __FUNCTION__);
    @synchronized(audioDataQueueMutex)
    {
        if (audioDataQueue != NULL && audioDataQueue->size() > 0)
        {
            std::vector<short> & vAudioData = audioDataQueue->front();
            inCompleteAQBuffer->mAudioDataByteSize = vAudioData.size() * sizeof(short);
            memcpy(inCompleteAQBuffer->mAudioData, &vAudioData[0], inCompleteAQBuffer->mAudioDataByteSize);
            //NSLog(@"...calling AudioQueueEnqueueBuffer with data...");
            OSStatus err = AudioQueueEnqueueBuffer(inAQ, inCompleteAQBuffer, 0, NULL);
            if (err != noErr)
            {
                NSLog(@"ERROR - AudioQueueEnqueueBuffer returned %d", (int)err);
            }
            audioDataQueue->pop_front();
        }
        else
        {
            //inCompleteAQBuffer->mAudioDataByteSize = 0L;
            //NSLog(@"... no audio data for queue to consume.");
            //[self pauseQueue];
            memset(inCompleteAQBuffer->mAudioData, 0, inCompleteAQBuffer->mAudioDataByteSize);
            OSStatus err = AudioQueueEnqueueBuffer(inAQ, inCompleteAQBuffer, 0, NULL);
            if (err != noErr)
            {
                NSLog(@"ERROR - AudioQueueEnqueueBuffer returned %d", (int)err);
            }
        }
    }
    mixer_buf_done_callback();
}

- (int) getNumPlaying
{
    if ( audioDataQueueMutex == nil)
        return 0;

    @synchronized(audioDataQueueMutex)
    {
        if (audioDataQueue != NULL)
            return audioDataQueue->size();
        else
            return 0;
    }
    return 0;
}

@end
