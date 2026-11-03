#ifdef DWYCO_IOS

#include "iosbackground.h"

#import <UIKit/UIKit.h>

IOSBackground::IOSBackground(QObject *parent) : QObject(parent)
{
}

quint64
IOSBackground::begin()
{
    __block IOSBackground *self = this;
    UIBackgroundTaskIdentifier tid =
        [[UIApplication sharedApplication] beginBackgroundTaskWithName:@"rando-send"
            expirationHandler:^{
                dispatch_async(dispatch_get_main_queue(), ^{
                    emit self->expired();
                });
            }];
    return (quint64)tid;
}

void
IOSBackground::end(quint64 taskId)
{
    [[UIApplication sharedApplication] endBackgroundTask:(UIBackgroundTaskIdentifier)taskId];
}

#endif
