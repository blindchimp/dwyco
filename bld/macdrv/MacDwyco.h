
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>

//#import "MacDwycoBlitCallbackDelegate.h"
#import "MacDwycoVideoSourceDelegate.h"
#import "MacDwycoAudioSource.h"

#define DWYCOCALLCONV
#define DWYCOEXPORT

/**
 * This class is a Cocoa-compatible wrapper on some of the Dwyco methods, plus
 * the implementation module for the library callbacks.
 *
 * Most functions are available as stateless class functions.  A singleton instance is
 * provided for registry of delegates which handle blitting received video data
 * to the screen (in the application's preferred context) and capturing video
 * data from a selected device.
 */
@interface MacDwyco : NSObject {
    NSMutableDictionary * blitCallbackDelegates;
    NSObject<MacDwycoVideoSourceDelegate> * videoSourceDelegate;
    MacDwycoAudioSource * audioSource;
    NSArray * videoInputDevices; // The cached list of video devices, as of the last time the library asked for them
    int currentVideoInputDeviceIndex;
}
+ (MacDwyco *) singleton;


/**
 * @section Instance methods for video capture and preview management,
 * as well as library callback implementation.
 */

/**
 * Initialize singleton instance; allocates private members.
 * @note If the singleton is already created this will return it instead of 'self'.
 */
- (id) init;

/**
 * Get the list of video input devices available.
 * @param refreshList If TRUE, query the system for available devices and cache
 * in the list; if FALSE, just return the cached list.
 * @return An NSArray of QTCaptureDevice objects.
 */
- (NSArray *) getVideoInputDevices:(BOOL) refreshList;

/**
 * Set the index of the video input device in use by the Dwyco library.
 * @param index The index.  If index is greater than or equal to number of devices, this call will be ignored.
 */
- (void) setCurrentVideoInputDeviceIndex:(int) index;

/**
 * Get the index of the current input video device.
 */
- (int) getCurrentVideoInputDeviceIndex;

/**
 * Get the current video input device.
 */
- (QTCaptureDevice *) getCurrentVideoInputDevice;



/**
 * Set the video source delegate.
 * @param delegate An object implementing the MacDwycoVideoSourceDelegate protocol.
 * @note Set to nil to unregister the current delegate.
 * @see MacDwycoVideoSourceDelegate
 * @see MacDwycoVideoSource
 */
- (void) setVideoSourceDelegate:(NSObject<MacDwycoVideoSourceDelegate> *) delegate;

/**
 * Get the video source delegate.
 */
- (NSObject<MacDwycoVideoSourceDelegate> *) getVideoSourceDelegate;

/**
 * Set the audio source.
 * @param source An MacDwycoAudioSource object.
 * @note Set to nil to unregister the current source.
 * @see MacDwycoAudioSource
 */
- (void) setAudioSource:(MacDwycoAudioSource *) source;

/**
 * Get the audio source.
 */
- (MacDwycoAudioSource *) getAudioSource;

@end

/**
 * @mainpage MacDwyco - Mac OS X Video Capture for the dwyco library.
 *
 * <p>The following classes make up the Mac implementation:</p>
 * <dl>
 * <dt>@ref MacDwyco</dt>
 * <dd>A singleton that represents the layer of abstraction between
 * Mac-specific implementation and types, and the dwyco dll's interface and types.</dd>
 * <dt>@ref MacDwycoVideoSource</dt>
 * <dd>A class implementing QTKit-based video capture and used by MacDwyco to furnish
 * vido frame data to the dwyco library.</dd>
 * <dt>@ref MacDwycoBlitCallbackDelegate</dt>
 * <dd>A protocol that a handling Mac native class must implement to display
 * the RGB video data obtained from the dwyco library.</dd>
 * <dt>@ref RGBPixelDataGLView</dt>
 * <dd>A subclass of NSOpenGLView which can be used in a Cocoa window to display
 * RGB video data in the format provided to a MacDwycoBlitCallbackDelegate.</dd>
 * </dl>
 */

