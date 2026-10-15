//
//  IOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#include "IOSOpenGLContextManager.hpp"


@interface MWKStimulusDisplayViewController : UIViewController

- (instancetype)initWithInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation;

@end


@implementation MWKStimulusDisplayViewController {
    UIInterfaceOrientation _interfaceOrientation;
}


- (instancetype)initWithInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    self = [super init];
    if (self) {
        _interfaceOrientation = interfaceOrientation;
    }
    return self;
}


- (BOOL)prefersStatusBarHidden {
    return YES;
}


- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    if (UIInterfaceOrientationIsPortrait(_interfaceOrientation)) {
        return (UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown);
    }
    return UIInterfaceOrientationMaskLandscape;
}


@end


BEGIN_NAMESPACE_MW


IOSOpenGLContextManager::IOSOpenGLContextManager() :
    metalDevice(nil)
 {
    @autoreleasepool {
        metalDevice = MTLCreateSystemDefaultDevice();
        if (!metalDevice) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Metal is not supported on this device");
        }
    }
}


IOSOpenGLContextManager::~IOSOpenGLContextManager() {
    @autoreleasepool {
        // Calling releaseContexts here causes the application to crash at exit.  Since this class is
        // used as a singleton, it doesn't matter, anyway.
        //releaseContexts();
        metalDevice = nil;
    }
}


int IOSOpenGLContextManager::newFullscreenContext(int screen_number, bool opaque) {
    //
    // NOTE: Since we only support the main display, we don't use the screen number.  Also, we
    // always make iOS windows opaque, as there's nothing to show through them from behind.
    //
    
    @autoreleasepool {
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            UIWindowScene *windowScene = nil;
            for (UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
                if ([scene isKindOfClass:[UIWindowScene class]]) {
                    windowScene = (UIWindowScene *)scene;
                    break;
                }
            }
            if (!windowScene) {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Application has no connected window scenes");
            } else if (UIWindow *window = [[UIWindow alloc] initWithWindowScene:windowScene]) {
                if (MWKStimulusDisplayViewController *viewController =
                    [[MWKStimulusDisplayViewController alloc]
                     initWithInterfaceOrientation:windowScene.effectiveGeometry.interfaceOrientation])
                {
                    window.rootViewController = viewController;
                    
                    if (MTKView *view = [[MTKView alloc] initWithFrame:window.bounds device:metalDevice]) {
                        viewController.view = view;
                        
                        [window makeKeyAndVisible];
                        
                        [views addObject:view];
                        [windows addObject:window];
                        
                        success = true;
                    }
                }
            }
        });
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create fullscreen window");
        }
        
        return (views.count - 1);
    }
}


int IOSOpenGLContextManager::newMirrorContext(double width, double height, int main_context_id) {
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror windows are not supported on this OS");
}


void IOSOpenGLContextManager::releaseContexts() {
    @autoreleasepool {
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (UIWindow *window in windows) {
                window.hidden = YES;
                window.windowScene = nil;
            }
            [windows removeAllObjects];
            [views removeAllObjects];
        });
    }
    
    AppleOpenGLContextManager::releaseContexts();
}


int IOSOpenGLContextManager::getNumDisplays() const {
    // At present, we support only the main display
    return 1;
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<IOSOpenGLContextManager>();
}


END_NAMESPACE_MW
