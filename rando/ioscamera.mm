#ifdef DWYCO_IOS

#include "ioscamera.h"

#include <QString>

#include "dlli.h"

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

@interface RandoCameraDelegate : NSObject <UIImagePickerControllerDelegate, UINavigationControllerDelegate>
@end

@implementation RandoCameraDelegate

- (void)imagePickerController:(UIImagePickerController *)picker
didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey, id> *)info
{
    UIImage *image = info[UIImagePickerControllerOriginalImage];
    if (image != nil) {
        NSData *jpeg = UIImageJPEGRepresentation(image, 0.9);
        if (jpeg != nil) {
            NSString *dir = NSTemporaryDirectory();
            [[NSFileManager defaultManager] createDirectoryAtPath:dir
                                     withIntermediateDirectories:YES
                                                      attributes:nil
                                                           error:nil];
            NSString *stamp = [[NSUUID UUID] UUIDString];
            NSString *filename = [NSString stringWithFormat:@"JPEG_%@.jpg", stamp];
            NSString *path = [dir stringByAppendingPathComponent:filename];
            BOOL ok = [jpeg writeToFile:path atomically:YES];
            if (ok) {
                dwyco_set_aux_string([path UTF8String]);
            } else {
                dwyco_set_aux_string("");
            }
        } else {
            dwyco_set_aux_string("");
        }
    } else {
        dwyco_set_aux_string("");
    }
    [picker dismissViewControllerAnimated:YES completion:nil];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    dwyco_set_aux_string("");
    [picker dismissViewControllerAnimated:YES completion:nil];
}

@end

static RandoCameraDelegate *randoCameraDelegate = nil;

static UIViewController *randoTopViewController(void)
{
    UIWindow *window = nil;
    if (@available(iOS 13.0, *)) {
        for (UIScene *scene in [UIApplication sharedApplication].connectedScenes) {
            if ([scene isKindOfClass:[UIWindowScene class]] &&
                scene.activationState == UISceneActivationStateForegroundActive) {
                UIWindowScene *ws = (UIWindowScene *)scene;
                for (UIWindow *w in ws.windows) {
                    if (w.isKeyWindow) {
                        window = w;
                        break;
                    }
                }
                if (window == nil && ws.windows.count > 0) {
                    window = ws.windows.firstObject;
                }
                break;
            }
        }
    }
    if (window == nil) {
        window = [UIApplication sharedApplication].keyWindow;
    }
    UIViewController *vc = window.rootViewController;
    while (vc != nil && vc.presentedViewController != nil) {
        vc = vc.presentedViewController;
    }
    return vc;
}

IOSCamera::IOSCamera(QObject *parent) : QObject(parent)
{
}

void IOSCamera::takePicture()
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera]) {
            dwyco_set_aux_string("");
            return;
        }
        if (randoCameraDelegate == nil) {
            randoCameraDelegate = [[RandoCameraDelegate alloc] init];
        }
        UIImagePickerController *picker = [[UIImagePickerController alloc] init];
        picker.sourceType = UIImagePickerControllerSourceTypeCamera;
        picker.delegate = randoCameraDelegate;
        UIViewController *vc = randoTopViewController();
        if (vc != nil) {
            [vc presentViewController:picker animated:YES completion:nil];
        } else {
            dwyco_set_aux_string("");
        }
    });
}

#endif
