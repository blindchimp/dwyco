
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
//  MacDwycoAudioSource.mm
//  DwycoVideoTest
//
//  Created by Christopher Corbell on 9/17/09.

#import "MacDwycoAudioSource.h"

namespace  {
const int kNumAudioQueueBuffers = 5;
}

static void local_audioInputCallback(void *                          inUserData, // a MacDwycoAudioSource *
                                     AudioQueueRef                   inAQ,
                                     AudioQueueBufferRef             inBuffer,
                                     const AudioTimeStamp *          inStartTime,
                                     UInt32                          inNumberPacketDescriptions,
                                     const AudioStreamPacketDescription *inPacketDescs)
{
    //NSLog(@"%s", __FUNCTION__);
    MacDwycoAudioSource * audioSource = (MacDwycoAudioSource *) inUserData;
    if (nil != audioSource)
    {
        [audioSource handleAudioInput:inAQ
         buffer:inBuffer
         time:inStartTime
         numPacketDescriptions:inNumberPacketDescriptions
         packetDescriptions:inPacketDescs];
    }
#if 0
    // note, if this happens, call qstop may cause a problem because
    // of locking that would need to happen, and this is acalled from another
    // thread.
    else
    {
        NSLog(@"ERROR - audio queue running but a nil audioSource came into handler; calling AudioQueueStop");
        if (NULL != inAQ)
        {
            AudioQueueStop(inAQ, TRUE);
        }
    }
#endif
}

static void local_audioPropertyListener(void *userData, AudioQueueRef queue, AudioQueuePropertyID propertyID)
{
    //MacDwycoAudioSource * audioSource = (MacDwycoAudioSource *)userData;
    if (propertyID == kAudioQueueProperty_IsRunning)
    {
        // Anything to do here?
        //NSLog(@"%s - audio queue is running", __FUNCTION__);
        //aqr->queueStartStopTime = CFAbsoluteTimeGetCurrent();
    }
}
#if 0
static void local_convertRawMono16bitPcmFileToWav(const char * tempFilePath,
        const char * wavFilePath,
        UInt32 dataLength)
{
    FILE * fpData = fopen(tempFilePath, "rb");
    FILE * fpWav = fopen(wavFilePath, "wb");

    fwrite("RIFF", 4, 1, fpWav);
    UInt32 fourByte = 4 + 24 + 8 + dataLength;
    fwrite(&fourByte, 4, 1, fpWav);
    fwrite("WAVE", 4, 1, fpWav);

    fwrite("fmt ", 4, 1, fpWav);
    fourByte = 16;
    fwrite(&fourByte, 4, 1, fpWav);
    UInt32 twoByte = 1; // PCM
    fwrite(&twoByte, 2, 1, fpWav);
    fwrite(&twoByte, 2, 1, fpWav); // 1 is also num channels
    fourByte = 44100; // sampling rate
    fwrite(&fourByte, 4, 1, fpWav);
    fourByte *= 2; // 2-bytes per sample * sample rate
    fwrite(&fourByte, 4, 1, fpWav);
    twoByte = 2;
    fwrite(&twoByte, 2, 1, fpWav); // 2 bytes per sample
    twoByte *= 8;
    fwrite(&twoByte, 2, 1, fpWav); // 16 bits per sample

    fwrite("data", 4, 1, fpWav);
    fwrite(&dataLength, 4, 1, fpWav);
    for (UInt32 nByte = 0; nByte < dataLength; nByte++)
    {
        fputc(fgetc(fpData), fpWav);
    }
    fclose(fpWav);
    fclose(fpData);

    // delete the temp file
    [[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithUTF8String:tempFilePath] handler:nil];
}
#endif


@implementation MacDwycoAudioSource

- (id) init
{
    //NSLog(@"%s", __FUNCTION__);
    self = [super init];
    if (nil != self)
    {
        packetSize = 7056;
        // This currently matches the DLL default, though the dll can change it later;
        // we really *should* simplify the capture code by making this always match the
        // audio queue buffer size - i.e. if the dll wants a different size, completely
        // re-initialize the audio queue to match

        meteringEnabled = YES; // TODO: change to false by default
        meterState.mPeakPower = meterState.mAveragePower = 0.0f;

        audioDataPackets = new std::deque<MacAudioPacket>;
        partialPacket = new MacAudioPacket;

        audioDataMutex = [[NSNumber numberWithInt:0] retain];

        OSStatus status = [self initializeAudioQueue];
        if (status != noErr)
        {
            NSLog(@"ERROR: Audio queue initialization returned %d", (int)status);
        }
        capturingFile = NO;
        audioCaptureFilePath = nil;
        captureFP = NULL;
    }
    return self;
}

#if 0
- (NSString *) description
{
    // Return a string representation of this object's state, for debugging / logging
    std::stringstream ss;

    return [NSString stringWithUTF8String:ss.str().c_str()];
}
#endif

- (void) dealloc
{
    //NSLog(@"%s", __FUNCTION__);
    [self off];
    // queue is running in an OS-managed thread; give it a brief time (100 ms) to shut down

    AudioQueueDispose(audioQueue, TRUE);
    [self clearDataPackets];
    delete audioDataPackets;
    delete partialPacket;
    [audioCaptureFilePath autorelease];
    [audioDataMutex autorelease];
    [super dealloc];
}

- (OSStatus) initializeAudioQueue
{
    //NSLog(@"%s", __FUNCTION__);
    audioStreamDesc.mFormatID = kAudioFormatLinearPCM;
    audioStreamDesc.mSampleRate = 44100.0;
    audioStreamDesc.mChannelsPerFrame = 1;
    audioStreamDesc.mBitsPerChannel = 16;
    audioStreamDesc.mBytesPerPacket = sizeof(SInt16);
    audioStreamDesc.mBytesPerFrame = sizeof(SInt16);
    audioStreamDesc.mFramesPerPacket = 1;
    audioStreamDesc.mFormatFlags =	kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;

    audioQueue = NULL;
    OSStatus err = AudioQueueNewInput(&audioStreamDesc,
                                      local_audioInputCallback,
                                      self,
                                      NULL,
                                      kCFRunLoopCommonModes,
                                      0,
                                      &audioQueue);
    if (err != noErr)
    {
        NSLog(@"AudioQueueNewInput returned %d", (int)err);
    }
    else if (NULL == audioQueue)
    {
        NSLog(@"AudioQueueNewInput return NULL audioQueue");
    }
    else
    {
        OSStatus allocErr = noErr;
        for (int i = 0; i < kNumAudioQueueBuffers && err == noErr; ++i)
        {
            AudioQueueBufferRef buffer;
            err = AudioQueueAllocateBuffer(audioQueue, packetSize, &buffer);
            if (err != noErr)
            {
                NSLog(@"AudioQueueAllocateBuffer returned %d", (int)allocErr);
            }
            else
            {
                err = AudioQueueEnqueueBuffer(audioQueue, buffer, 0, NULL);
                if (err != noErr)
                {
                    NSLog(@"AudioQueueEnqueueBuffer returned %d", (int)allocErr);
                }
            }
        }
        if (err == noErr)
        {
            // Enable metering by default
            UInt32 enableMetering = 1;
            err = AudioQueueSetProperty(audioQueue,
                                        kAudioQueueProperty_EnableLevelMetering,
                                        &enableMetering,
                                        sizeof(enableMetering));
        }
    }
    return err;
}

- (BOOL) isQueueRunning
{
    return self->queueRunning;
}

- (BOOL) hasCaptureData
{
    BOOL hasData = NO;
    @synchronized(audioDataMutex)
    {
        hasData = (!audioDataPackets->empty());
    }
    return hasData;
}

- (void) on
{
    // TODO: zero audioDataSampleTime?
    //NSLog(@"%s", __FUNCTION__);
    if (!queueRunning)
    {
        //NSLog(@"calling AudioQueueStart");
        OSStatus err = AudioQueueStart(audioQueue, NULL);
        if (err != noErr)
        {
            NSLog(@"ERROR: AudioQueueStart returned %d", (int)err);

        }
        else
        {
            queueRunning = YES;
        }
    }
    else
    {
        //NSLog(@"- queue already running, no-op");
    }
}

// not sure if this is used by the dll
- (void) pause
{
    //NSLog(@"%s", __FUNCTION__);
    OSStatus err = AudioQueuePause(audioQueue);
    if (err != noErr)
    {
        NSLog(@"ERROR: AudioQueueStop returned %d", (int)err);
    }
    //AudioQueueFlush(audioQueue);
    //[self stopAudioFileCapture];
    //[self clearDataPackets];
    queueRunning = NO;
}

- (void) off
{
    //NSLog(@"%s", __FUNCTION__);
    OSStatus err = AudioQueuePause(audioQueue);
    if (err != noErr)
    {
        NSLog(@"ERROR: AudioQueuePaise returned %d", (int)err);
    }
    //AudioQueueFlush(audioQueue);
    [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    //[self stopAudioFileCapture];
    [self clearDataPackets];
    queueRunning = NO;
}

- (void) reset
{
    //NSLog(@"%s", __FUNCTION__);
    OSStatus err = AudioQueueStop(audioQueue, TRUE);
    if (err != noErr)
    {
        NSLog(@"AudioQueueReset returned %d", (int)err);
    }
    //[self stopAudioFileCapture];
    [self clearDataPackets];
}

- (MacAudioPacket) popLatestAudioPacket
{
    MacAudioPacket packet;
    @synchronized(audioDataMutex)
    {
        if (audioDataPackets->size() > 0)
        {
            packet = audioDataPackets->front();
            audioDataPackets->pop_front();
        }
    }
    return packet;
}

- (int) packetSize
{
    return packetSize;
}

- (void) setPacketSize:(int) nSize
{
    // TODO: we may want to re-initialize the AudioQueue so that capture buffer size
    // is always equal to packet size.
    packetSize = nSize;
}

- (BOOL) isMeteringEnabled
{
    return meteringEnabled && queueRunning;
}

- (void) setMeteringEnabled:(BOOL) bYesNo
{
    if (bYesNo != meteringEnabled)
    {
        // Enable metering by default
        UInt32 enableMetering = bYesNo ? 1 : 0;
        OSStatus err = AudioQueueSetProperty(audioQueue,
                                             kAudioQueueProperty_EnableLevelMetering,
                                             &enableMetering,
                                             sizeof(enableMetering));
        if (err != noErr)
        {
            NSLog(@"AudioQueueSetProperty(kAudioQueueProperty_EnableLevelMetering) return %d", (int)err);
        }
        meteringEnabled = bYesNo;
    }
}

- (float) getMeterAveragePower
{
    if (!queueRunning)
        return 0.0f;

    UInt32 propertySize = sizeof (AudioQueueLevelMeterState);

    OSStatus err = AudioQueueGetProperty (audioQueue,
                                          (AudioQueuePropertyID) kAudioQueueProperty_CurrentLevelMeter,
                                          &meterState,
                                          &propertySize
                                         );

    //NSLog(@"AudioQueueGetProperty returned %ld; avg power is %f", err, meterState.mAveragePower);
    return meterState.mAveragePower;
}

- (void) setAudioCaptureFile:(NSString *) filePath
{
    if (nil != filePath)
        audioCaptureFilePath = [[NSString stringWithString:filePath] retain];
    else
        audioCaptureFilePath = nil;
}

- (void) startAudioFileCapture
{
    //NSLog(@"%s", __FUNCTION__);
    if (capturingFile)
    {
        NSLog(@"startAudioFileCapture - already capturing, ignoring");
        return;
    }

    if (!queueRunning)
    {
        NSLog(@"startAudioFileCapture - queue is not running, ignoring");
        return;
    }

    if (nil == audioCaptureFilePath || [audioCaptureFilePath length] == 0)
    {
        [self setAudioCaptureFile:@"temp.wav"];
    }

    @synchronized(audioDataMutex)
    {
        NSString * tempFilePath = [audioCaptureFilePath stringByAppendingString:@".temp"];
        captureFP = fopen([tempFilePath UTF8String], "wb");
        capturingFile = TRUE;
        captureFileByteCount = 0;
    }
}

#if 0
- (void) stopAudioFileCapture
{
    //NSLog(@"%s", __FUNCTION__);
    if (capturingFile)
    {
        if (NULL != captureFP)
        {
            NSString * tempFilePath = nil;
            if (nil != audioCaptureFilePath)
            {
                tempFilePath = [audioCaptureFilePath stringByAppendingString:@".temp"];
            }
            NSString * filePath = audioCaptureFilePath;
            UInt32 dataLength = 0;
            @synchronized(audioDataMutex)
            {
                capturingFile = FALSE;
                if (captureFP != NULL)
                    fclose(captureFP);
                captureFP = NULL;
                dataLength = captureFileByteCount;
                captureFileByteCount = 0;
            }

            if (nil != tempFilePath && nil != filePath && dataLength > 0)
                local_convertRawMono16bitPcmFileToWav([tempFilePath UTF8String], [filePath UTF8String], dataLength);
        }
        else
        {
            // Make sure other state variables are cleared
            capturingFile = FALSE;
            captureFileByteCount = 0;
        }
    }
}
#endif


- (void) handleAudioInput:(AudioQueueRef) inAQ
    buffer:(AudioQueueBufferRef) inBuffer
    time:(const AudioTimeStamp *) inStartTime
    numPacketDescriptions:(UInt32) inNumberPacketDescriptions
    packetDescriptions:(const AudioStreamPacketDescription *) inPacketDescs
{
    if (inNumberPacketDescriptions > 0 && inBuffer->mAudioDataByteSize > 0L)
    {
        @synchronized(audioDataMutex)
        {
            int bytesRemaining = inBuffer->mAudioDataByteSize;
            //NSLog(@"%s - sampleTime is %.12lg, bytes: %d", __FUNCTION__, inStartTime->mSampleTime, bytesRemaining);

            int copySrcOffset = 0;
            int packetCount = 0;
            int framesPerPacket = packetSize / sizeof(SInt16);

            while (bytesRemaining > 0)
            {
                MacAudioPacket packet;

                if (partialPacket->dataSize != 0) // we need to finish filling up a partial packet
                {
                    // copy partial packet member
                    packet = *partialPacket;

                    // zero out partial packet member
                    partialPacket->data = NULL;
                    partialPacket->dataSize = 0L;
                }
                else // start a new packet
                {
                    if (packet.data == NULL)
                    {
                        packet.data = new char[packetSize];
                    }
                    packet.sampleTime = inStartTime->mSampleTime + (Float64) (framesPerPacket * packetCount);
                }

                // bytes to copy - make sure it's less than available packet buffer, and <= available bytes
                int bytesToCopy = packetSize - packet.dataSize;
                if (bytesToCopy > bytesRemaining)
                    bytesToCopy = bytesRemaining;

                bool savePartialPacket = (packet.dataSize + bytesToCopy < packetSize);
                memcpy(packet.data + packet.dataSize, ((char *) inBuffer->mAudioData) + copySrcOffset, bytesToCopy);
                packet.dataSize += bytesToCopy;

                bytesRemaining -= bytesToCopy;
                copySrcOffset += bytesToCopy;

                packetCount++;

                if (savePartialPacket)
                {
                    //NSLog(@"saving partial packet...");
                    *partialPacket = packet;
                    break; // shouldn't be necessary but just to be safe
                }
                else
                {
                    //NSLog(@"Pushing back packet: %s", packet.ToString().c_str());
                    audioDataPackets->push_back(packet);
                }
            }

            if (capturingFile && NULL != captureFP)
            {
                fwrite(inBuffer->mAudioData, 1, inBuffer->mAudioDataByteSize, captureFP);
                captureFileByteCount += inBuffer->mAudioDataByteSize;
            }
        }
    }
    else
    {
        //NSLog(@"%s - no audio data, no-op", __FUNCTION__);
    }

    // Enqueue the input buffer for reuse if queue is still running
    // (It's possible that we're getting the last flushed buffers after
    // the queue has been stopped).
    if (self->queueRunning)
    {
        AudioQueueEnqueueBuffer(audioQueue, inBuffer,  0, NULL);
    }
}

- (void) clearDataPackets
{
    @synchronized(audioDataMutex)
    {
        // any unretrieved packets are cleared at this point
        while (audioDataPackets->size() > 0)
        {
            MacAudioPacket packet = audioDataPackets->front();
            audioDataPackets->pop_front();
            if (NULL != packet.data)
                delete [] packet.data;
        }
        if (NULL != partialPacket && NULL != partialPacket->data)
        {
            delete partialPacket->data;
            partialPacket->data = NULL;
            partialPacket->dataSize = 0L;
        }
    }
}

- (MacAudioPacket *) partialPacket
{
    return partialPacket;
}

@end

