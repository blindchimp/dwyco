
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import "MacAudioOutputBase.h"

#include <deque>
#include <vector>

@interface MacDwycoAudioOutput : MacAudioOutputBase {

    NSObject *							audioDataQueueMutex;
    std::deque< std::vector<short> > *	audioDataQueue;
}
- (id) init;
- (void) dealloc;

//+ (void) initializeDwycoCallbacks;

+ (MacDwycoAudioOutput *) getSharedOutput; // singleton accessor/allocator
+ (void) releaseSharedOutput; // singleton destroyer

// Invoked from dwyco server callback: add received audio data to this object's
// queue, where it will wait for the AudioQueue callback to consume it.
// This call is multi-thread-safe.
- (void) pushDwycoReceivedAudioData:(char *) data length:(int) length;

// Push silence - useful when queue is first starting while waiting for server audio
- (void) pushSilence:(int) numSamples;

// Invoked from dwyco server reset callback: clear out any pending data from the server.
// (This call does not change the state of the AudioQueue run loop, it just removes pending data).
// This call is multi-thread-safe.
- (void) resetDwycoAudioData;

// This override appends data to the output queue based on what's been
// accumulated in the intermediate buffer
- (void) outputCallback:(AudioQueueRef)inAQ buffer:(AudioQueueBufferRef) inCompleteAQBuffer;

- (int) getNumPlaying;

@end
