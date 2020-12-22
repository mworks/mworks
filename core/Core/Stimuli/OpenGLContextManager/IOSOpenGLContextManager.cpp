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
@end


@implementation MWKStimulusDisplayViewController


- (BOOL)prefersStatusBarHidden {
    return YES;
}


- (BOOL)shouldAutorotate {
    return NO;
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
        if (screen_number < 0 || screen_number >= getNumDisplays()) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Invalid screen number (%d)") % screen_number).str());
        }
        
        MWKOpenGLContext *context = [[MWKOpenGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if (!context) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL ES context");
        }
        
        auto screen = UIScreen.screens[screen_number];
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            if (UIWindow *window = [[UIWindow alloc] initWithFrame:screen.bounds]) {
                if (window.screen != screen) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN, "Window is not on the requested screen");
                } else if (MWKStimulusDisplayViewController *viewController = [[MWKStimulusDisplayViewController alloc] init]) {
                    window.rootViewController = viewController;
                    
                    if (MTKView *view = [[MTKView alloc] initWithFrame:window.bounds device:metalDevice]) {
                        viewController.view = view;
                        
                        [window makeKeyAndVisible];
                        
                        [contexts addObject:context];
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
        
        return (contexts.count - 1);
    }
}


int IOSOpenGLContextManager::newMirrorContext(int main_context_id) {
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
        
        [contexts removeAllObjects];
    }
}


int IOSOpenGLContextManager::getNumDisplays() const {
    // At present, we support only the main display
    return 1;
}


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    @autoreleasepool {
        if (auto context = getContext(context_id)) {
            if ([EAGLContext setCurrentContext:context]) {
                return [context lockContext];
            }
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot set current OpenGL ES context");
        }
        return OpenGLContextLock();
    }
}


void IOSOpenGLContextManager::clearCurrent() {
    @autoreleasepool {
        [EAGLContext setCurrentContext:nil];
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<IOSOpenGLContextManager>();
}


END_NAMESPACE_MW
