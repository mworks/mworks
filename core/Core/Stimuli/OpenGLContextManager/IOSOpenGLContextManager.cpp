//
//  IOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#include "IOSOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"

#include <Metal/Metal.h>


@interface MWKMetalViewController : UIViewController
@end


@implementation MWKMetalViewController


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
                window.screen = screen;
                
                if (MWKMetalViewController *viewController = [[MWKMetalViewController alloc] init]) {
                    window.rootViewController = viewController;
                    
                    if (MWKMetalView *view = [[MWKMetalView alloc] initWithFrame:window.bounds device:metalDevice]) {
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


int IOSOpenGLContextManager::newMirrorContext() {
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror windows are not supported on this OS");
}


void IOSOpenGLContextManager::releaseContexts() {
    @autoreleasepool {
        releaseFramebufferTextures();
        
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


void IOSOpenGLContextManager::prepareContext(int context_id, bool useColorManagement) {
    @autoreleasepool {
        if (auto view = static_cast<MWKMetalView *>(getView(context_id))) {
            __block bool success = false;
            
            dispatch_sync(dispatch_get_main_queue(), ^{
                NSError *error = nil;
                if (![view prepareUsingColorManagement:useColorManagement error:&error]) {
                    merror(mw::M_DISPLAY_MESSAGE_DOMAIN,
                           "Cannot prepare Metal view: %s",
                           error.localizedDescription.UTF8String);
                } else {
                    if (useColorManagement) {
                        auto displayGamut = view.window.screen.traitCollection.displayGamut;
                        switch (displayGamut) {
                            case UIDisplayGamutSRGB:
                                // No color conversion required
                                break;
                                
                            case UIDisplayGamutP3: {
                                //
                                // According to "What's New in Metal, Part 2" (WWDC 2016, Session 605), applications should
                                // always render in the sRGB colorspace, even when the target device has a P3 display.  To use
                                // colors outside of the sRGB gamut, the app needs to use a floating-point color buffer and
                                // encode the P3-only colors using component values less than 0 or greater than 1 (as in Apple's
                                // "extended sRGB" color space).  Since MWKMetalView uses an 8 bit per channel, integer color
                                // buffer, we're always limited to sRGB.
                                //
                                // Testing suggests that this approach is correct.  If we draw an image with high color
                                // saturation and then display it both with and without a Mac-style, LUT-based conversion
                                // from sRGB to Display P3, the unconverted colors match what we see on a Mac, while the converted
                                // colors are noticeably duller.  This makes sense, because converting, say, 100% red (255, 0, 0)
                                // in sRGB to the wider gamut of Display P3 results in smaller numerical values (234, 51, 35).
                                //
                                // https://developer.apple.com/videos/play/wwdc2016/605/
                                //
                                break;
                            }
                                
                            default:
                                mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Unknown display gamut (%ld)", displayGamut);
                                break;
                        }
                    }
                    success = true;
                }
            });
            
            if (!success) {
                throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot prepare OpenGL context");
            }
        }
    }
}


END_NAMESPACE_MW
