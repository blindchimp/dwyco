
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
//  MacDwycoCameraSettingsController.mm
//  DwycoVideoTest
//
//  Created by Christopher Corbell on 9/1/09.
//

#import "MacDwycoCameraSettingsController.h"
#import "MacDwycoVideoSource.h"

static MacDwycoCameraSettingsController * sharedSettingsController = nil;

@implementation MacDwycoCameraSettingsController
+ (void) showPanelForSource:(MacDwycoVideoSource *) source
{
    if (nil == sharedSettingsController)
    {
        sharedSettingsController = [[MacDwycoCameraSettingsController alloc] initWithVideoSource:source];
        if (![NSBundle loadNibNamed:@"CameraSettings" owner:sharedSettingsController])
        {
            [sharedSettingsController autorelease];
            sharedSettingsController = nil;
            NSBeep();
            NSLog(@"ERROR: failed to load camera settings nib resource.");
        }
    }
    [sharedSettingsController showPanel];
}

+ (void) releaseSharedController
{
    if (nil != sharedSettingsController)
    {
        MacDwycoCameraSettingsController * temp;
        @synchronized(sharedSettingsController) {
            temp = sharedSettingsController;
            sharedSettingsController = nil;
        }
        [temp release];
    }
}

- (id) initWithVideoSource:(MacDwycoVideoSource *) source
{
    self = [super init];
    if (nil != self)
    {
        videoSource = source;
        [videoSource retain];
    }
    return self;
}

- (void) dealloc
{
    [cameraSettingsPanel close];

    // needed?
    //[cameraSettingsPanel release];

    [videoSource release];
    [super dealloc];
}

- (void) awakeFromNib
{
    [self loadSettingsFromSource];
}

- (void) loadSettingsFromSource
{
    [brightnessSlider setFloatValue:[videoSource brightness]];
    [contrastSlider setFloatValue:[videoSource contrast]];
    [saturationSlider setFloatValue:[videoSource saturation]];
    [gammaSlider setFloatValue:[videoSource gamma]];
    [flipImageVerticallyCheckbox setState:([videoSource flipImageVertically] ? NSOnState : NSOffState)];
}

- (void) showPanel
{
    [cameraSettingsPanel setIsVisible:YES];
}

- (IBAction) adjustBrightness:(id) sender
{
    [videoSource setBrightness:[brightnessSlider floatValue]];
}

- (IBAction) adjustContrast:(id) sender
{
    [videoSource setContrast:[contrastSlider floatValue]];
}

- (IBAction) adjustSaturation:(id) sender
{
    [videoSource setSaturation:[saturationSlider floatValue]];
}

- (IBAction) adjustGamma:(id) sender
{
    [videoSource setGamma:[gammaSlider floatValue]];
}

- (IBAction) flipImageVertically:(id) sender
{
    [videoSource setFlipImageVertically:(BOOL)[flipImageVerticallyCheckbox intValue]];
}

- (IBAction) restoreDefaults:(id) sender
{
    [videoSource restoreDefaultSettings];
    [videoSource saveSettingsToPreferences];
    [self loadSettingsFromSource];
}

- (IBAction) saveSettings:(id) sender
{
    [videoSource saveSettingsToPreferences];
}

@end
