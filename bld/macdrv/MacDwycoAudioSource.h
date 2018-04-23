
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
//  MacDwycoAudioSource.h
//  DwycoVideoTest
//
//  Created by Christopher Corbell on 9/17/09.

#import <Cocoa/Cocoa.h>

#import "MacAudioPacket.h"

#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudioTypes.h>

#include <deque>

@interface MacDwycoAudioSource : NSObject {

    AudioQueueRef					audioQueue;
    AudioStreamBasicDescription		audioStreamDesc;

    int								packetSize;

    std::deque<MacAudioPacket> *	audioDataPackets;
    MacAudioPacket *				partialPacket;

    NSObject *						audioDataMutex;

    BOOL							meteringEnabled;
    AudioQueueLevelMeterState		meterState;

    BOOL							capturingFile;
    NSString *						audioCaptureFilePath;
    FILE *							captureFP;
    UInt32							captureFileByteCount;

    BOOL							queueRunning;
}
// TODO: add support for enumerating audio input devices and initializing with
// a device other than the default input device, with kAudioQueueProperty_CurrentDevice

- (id) init;
- (void) dealloc;

- (BOOL) isQueueRunning;

- (OSStatus) initializeAudioQueue;

- (BOOL)		hasCaptureData;

- (void)		on;
- (void)		pause;
- (void)		off;
- (void)		reset;

/** Get latest audio frame.
 * @return An AudioFrame structure.  If there is no latest frame, the structure's
 *         data field will be NULL.
 */
- (MacAudioPacket) popLatestAudioPacket;

- (int)			packetSize;
- (void)		setPacketSize:(int) nSize;

- (BOOL)		isMeteringEnabled;
- (void)		setMeteringEnabled:(BOOL) bYesNo;
- (float)		getMeterAveragePower;

//- (void)		setAudioCaptureFile:(NSString *) filePath;
//- (void)		startAudioFileCapture;
//- (void)		stopAudioFileCapture;

// Callback to process audio input
- (void) handleAudioInput:(AudioQueueRef) inAQ
    buffer:(AudioQueueBufferRef) inBuffer
    time:(const AudioTimeStamp *) inStartTime
    numPacketDescriptions:(UInt32) inNumberPacketDescriptions
    packetDescriptions:(const AudioStreamPacketDescription *) inPacketDescs;

/**
 * Delete any unretrieved data packets in the queue, as well as
 * the buffer for the partial packet; this is a synchronized method.
 */
- (void)		clearDataPackets;

- (MacAudioPacket *) partialPacket;

@end
