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
#ifdef DWYCO_IOS_BG_TEST
    dispatch_async(dispatch_get_main_queue(), ^{
        // Test shim: pretend a large image was captured so the send takes
        // long enough to background the app and exercise the iOS background path.
        int w = 2560, h = 1920;
        size_t bytesPerRow = w * 4;
        unsigned char *pixels = (unsigned char *)malloc(bytesPerRow * h);
        // Fill with high-entropy noise so the JPEG is large and slow to upload.
        for (size_t i = 0; i < bytesPerRow * h; i++) {
            pixels[i] = (unsigned char)(arc4random_uniform(256));
        }
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx = CGBitmapContextCreate(pixels, w, h, 8, bytesPerRow, cs, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
        UIImage *image = [UIImage imageWithCGImage:cgImage];
        CGImageRelease(cgImage);
        CGContextRelease(ctx);
        CGColorSpaceRelease(cs);
        free(pixels);
        NSData *jpeg = UIImageJPEGRepresentation(image, 0.95);
        NSString *path = [NSTemporaryDirectory() stringByAppendingPathComponent:@"TEST_bg.jpg"];
        [jpeg writeToFile:path atomically:YES];
        dwyco_set_aux_string([path UTF8String]);
    });
    return;
#endif
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
