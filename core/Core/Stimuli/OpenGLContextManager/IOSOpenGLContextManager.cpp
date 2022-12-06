//
//  IOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#include "IOSOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"


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
    // NOTE: We always make iOS windows opaque, as there's nothing to show through them from behind
    //
    
    @autoreleasepool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        auto screen = UIScreen.screens[screen_number];
#pragma clang diagnostic pop
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            if (UIWindow *window = [[UIWindow alloc] initWithFrame:screen.bounds]) {
                if (window.screen != screen) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN, "Window is not on the requested screen");
                } else if (MWKStimulusDisplayViewController *viewController =
                           [[MWKStimulusDisplayViewController alloc] initWithInterfaceOrientation:window.windowScene.interfaceOrientation])
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


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    return OpenGLContextLock();
}


void IOSOpenGLContextManager::clearCurrent() {
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<IOSOpenGLContextManager>();
}


END_NAMESPACE_MW
