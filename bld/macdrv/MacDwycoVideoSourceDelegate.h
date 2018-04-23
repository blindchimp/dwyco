
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/*
 *  MacDwycoVideoSourceDelegate.h
 *  DwycoVideoTest
 *
 */

/**
 * Video source delegate protocol, for object that is
 * furnishing video to the library.
 */
@protocol MacDwycoVideoSourceDelegate
- (BOOL)		hasCaptureData;
- (void)		startCapture;
- (void)		stopCapture;
- (NSData *)	getLatestCapturedRGBFrameBytes96x128;
- (NSData *)	getLatestCapturedRGBFrameBytes: (int *)width height:(int *)height;
- (CVImageBufferRef) getLatestCapturedRGBImageBuffer: (int *)width height:(int *)height;
- (QTTime)		lastCaptureTime;
- (void)		showSourceDialog;
- (void)		previewOn;
- (void)		previewOff;
- (BOOL)		flipImageVertically; /* does the user want the image flipped? */
- (void)		setCaptureSize: (int) width height:(int) height;
- (int)			isCapturing;
@end
