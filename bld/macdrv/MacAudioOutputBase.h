
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import <Cocoa/Cocoa.h>

#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudioTypes.h>

@interface MacAudioOutputBase : NSObject {
    AudioQueueRef					audioQueue;
    AudioStreamBasicDescription		audioStreamDesc;

    id								delegate;

    BOOL							queueRunning;
}
- (id) init; // Init with default audio output device
- (void) dealloc;

- (void) setDelegate:(id) delegate;

- (OSStatus) startQueue;
- (OSStatus) flushQueue;
- (OSStatus) pauseQueue;
- (OSStatus) stopQueue;

- (BOOL) isQueueRunning;

- (AudioQueueRef) audioQueue;

// Supply audio data to queue in callback.
//
// If this object has no delegate, this method will simply log.
//
// To actually provide data to the output queue, implement a method with this exact
// signature another class and set an instance of that class as a delegate
// to an instance of this class.
//
// Alternately, this class may be subclassed and this method overridden.

- (void) outputCallback:(AudioQueueRef)inAQ buffer:(AudioQueueBufferRef) inCompleteAQBuffer;
@end
