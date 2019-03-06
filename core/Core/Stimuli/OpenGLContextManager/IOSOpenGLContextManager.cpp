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


@interface MWKEAGLContext : EAGLContext

- (mw::OpenGLContextLock)lockContext;

@end


@implementation MWKEAGLContext {
    mw::OpenGLContextLock::unique_lock::mutex_type mutex;
}


- (mw::OpenGLContextLock)lockContext {
    return mw::OpenGLContextLock(mw::OpenGLContextLock::unique_lock(mutex));
}


@end


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


IOSOpenGLContextManager::IOSOpenGLContextManager() {
    @autoreleasepool {
        metalDevice = MTLCreateSystemDefaultDevice();
        if (!metalDevice) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Metal is not supported on this device");
        }
    }
}


IOSOpenGLContextManager::~IOSOpenGLContextManager() {
    // Calling releaseContexts here causes the application to crash at exit.  Since this class is
    // used as a singleton, it doesn't matter, anyway.
    //releaseContexts();
}


int IOSOpenGLContextManager::newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) {
    //
    // NOTE: We always make iOS windows opaque, as there's nothing to show through them from behind
    //
    
    @autoreleasepool {
        if (screen_number < 0 || screen_number >= getNumDisplays()) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Invalid screen number (%d)") % screen_number).str());
        }
        
        EAGLSharegroup *sharegroup = nil;
        if (contexts.count > 0) {
            sharegroup = contexts[0].sharegroup;
        }
        
        MWKEAGLContext *context = [[MWKEAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];
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
                        view.contentScaleFactor = (render_at_full_resolution ? screen.nativeScale : 1.0);
                        [EAGLContext setCurrentContext:context];
                        
                        if ([view prepare]) {
                            [window makeKeyAndVisible];
                            
                            [contexts addObject:context];
                            [views addObject:view];
                            [windows addObject:window];
                            
                            success = true;
                        }
                        
                        [EAGLContext setCurrentContext:nil];
                        [view release];
                    }
                    
                    [viewController release];
                }
                
                [window release];
            }
        });
        
        [context release];
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create fullscreen window");
        }
        
        return (contexts.count - 1);
    }
}


int IOSOpenGLContextManager::newMirrorContext(bool render_at_full_resolution) {
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
        if (auto context = static_cast<MWKEAGLContext *>(getContext(context_id))) {
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
    if (useColorManagement) {
        checkDisplayGamut(context_id);
    }
}


void IOSOpenGLContextManager::checkDisplayGamut(int context_id) const {
    @autoreleasepool {
        if (auto view = getView(context_id)) {
            __block UIDisplayGamut displayGamut;
            dispatch_sync(dispatch_get_main_queue(), ^{
                displayGamut = view.window.screen.traitCollection.displayGamut;
            });
            
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
                    // "extended sRGB" color space).  Since StimulusDisplay uses an 8 bit per channel, integer color
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
                    merror(M_DISPLAY_MESSAGE_DOMAIN, "Unsupported display gamut");
                    break;
            }
        }
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<IOSOpenGLContextManager>();
}


END_NAMESPACE_MW
