//
//  MacOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/6/19.
//

#include "MacOSOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"


BEGIN_NAMESPACE_MW


MacOSOpenGLContextManager::MacOSOpenGLContextManager() :
    display_sleep_block(kIOPMNullAssertionID)
{ }


MacOSOpenGLContextManager::~MacOSOpenGLContextManager() {
    // Calling releaseContexts here causes the application to crash at exit.  Since this class is
    // used as a singleton, it doesn't matter, anyway.
    //releaseContexts();
}


static MWKOpenGLContext * createOpenGLContext() {
    MWKOpenGLContext *context = nil;
    
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAAccelerated,
        0
    };
    
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    if (pixelFormat) {
        context = [[MWKOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
        if (context) {
            // Crash on calls to functions removed from the core profile
            CGLEnable(context.CGLContextObj, kCGLCECrashOnRemovedFunctions);
        }
    }
    
    return context;
}


static id<MTLDevice> getMetalDeviceForScreen(NSScreen *screen) {
    NSNumber *screenNumber = screen.deviceDescription[@"NSScreenNumber"];
    CGDirectDisplayID displayID = screenNumber.unsignedIntValue;
    id<MTLDevice> metalDevice = CGDirectDisplayCopyCurrentMetalDevice(displayID);
    if (!metalDevice) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Metal is not supported on selected display");
    }
    return metalDevice;
}


int MacOSOpenGLContextManager::newFullscreenContext(int screen_number, bool opaque) {
    @autoreleasepool {
        MWKOpenGLContext *context = createOpenGLContext();
        if (!context) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for fullscreen window");
        }
        
        NSScreen *screen = NSScreen.screens[screen_number];
        NSRect frame = NSMakeRect(0.0, 0.0, screen.frame.size.width, screen.frame.size.height);
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            if (NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                               styleMask:NSWindowStyleMaskBorderless
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO
                                                                  screen:screen])
            {
                window.level = NSMainMenuWindowLevel + 1;
                window.opaque = opaque;
                if (!opaque) {
                    window.backgroundColor = NSColor.clearColor;
                }
                window.hidesOnDeactivate = NO;
                
                if (MTKView *view = [[MTKView alloc] initWithFrame:frame
                                                            device:getMetalDeviceForScreen(screen)])
                {
                    window.contentView = view;
                    view.layer.opaque = opaque;
                    
                    [window orderFront:nil];
                    
                    [contexts addObject:context];
                    [views addObject:view];
                    [windows addObject:window];
                    
                    success = true;
                }
            }
        });
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create fullscreen window");
        }
        
        if (kIOPMNullAssertionID == display_sleep_block) {
            if (kIOReturnSuccess != IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep,
                                                                kIOPMAssertionLevelOn,
                                                                (CFStringRef)@"MWorks Prevent Display Sleep",
                                                                &display_sleep_block)) {
                mwarning(M_SERVER_MESSAGE_DOMAIN, "Cannot disable display sleep");
            }
        }
        
        return (contexts.count - 1);
    }
}


int MacOSOpenGLContextManager::newMirrorContext(double width, double height, int main_context_id) {
    @autoreleasepool {
        MWKOpenGLContext *context = createOpenGLContext();
        if (!context) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for mirror window");
        }
        
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            if (NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(50.0, 50.0, width, height)
                                                               styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable)
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO])
            {
                MTKView *mainView = nil;
                id<MTLDevice> metalDevice = nil;
                if (NO_CONTEXT_ID != main_context_id && (mainView = getView(main_context_id))) {
                    metalDevice = mainView.device;
                } else {
                    metalDevice = getMetalDeviceForScreen(window.screen);
                }
                
                if (MTKView *view = [[MTKView alloc] initWithFrame:NSMakeRect(0.0, 0.0, width, height)
                                                            device:metalDevice])
                {
                    window.contentView = view;
                    [window setFrameAutosaveName:[NSString stringWithFormat:@"OpenGLContextManager Window %lu",
                                                  windows.count]];
                    
                    [window orderFront:nil];
                    
                    [contexts addObject:context];
                    [views addObject:view];
                    [windows addObject:window];
                    
                    success = true;
                }
            }
        });
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create mirror window");
        }
        
        return (contexts.count - 1);
    }
}


void MacOSOpenGLContextManager::releaseContexts() {
    @autoreleasepool {
        if (kIOPMNullAssertionID != display_sleep_block) {
            (void)IOPMAssertionRelease(display_sleep_block);  // Ignore the return code
            display_sleep_block = kIOPMNullAssertionID;
        }
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (NSWindow *window in windows) {
                [window orderOut:nil];
            }
            [windows removeAllObjects];
            [views removeAllObjects];
        });
        
        [contexts removeAllObjects];
    }
    
    AppleOpenGLContextManager::releaseContexts();
}


int MacOSOpenGLContextManager::getNumDisplays() const {
    @autoreleasepool {
        return NSScreen.screens.count;
    }
}


OpenGLContextLock MacOSOpenGLContextManager::setCurrent(int context_id) {
    @autoreleasepool {
        if (auto context = getContext(context_id)) {
            [context makeCurrentContext];
            return [context lockContext];
        }
        return OpenGLContextLock();
    }
}


void MacOSOpenGLContextManager::clearCurrent() {
    @autoreleasepool {
        [NSOpenGLContext clearCurrentContext];
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<MacOSOpenGLContextManager>();
}


END_NAMESPACE_MW
