//
//  MacOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/6/19.
//

#include "MacOSOpenGLContextManager.hpp"


BEGIN_NAMESPACE_MW


MacOSOpenGLContextManager::MacOSOpenGLContextManager() :
    display_sleep_block(kIOPMNullAssertionID)
{ }


MacOSOpenGLContextManager::~MacOSOpenGLContextManager() {
    // Calling releaseContexts here causes the application to crash at exit.  Since this class is
    // used as a singleton, it doesn't matter, anyway.
    //releaseContexts();
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
        
        return (views.count - 1);
    }
}


int MacOSOpenGLContextManager::newMirrorContext(double width, double height, int main_context_id) {
    @autoreleasepool {
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
                    
                    [views addObject:view];
                    [windows addObject:window];
                    
                    success = true;
                }
            }
        });
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create mirror window");
        }
        
        return (views.count - 1);
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
    }
    
    AppleOpenGLContextManager::releaseContexts();
}


int MacOSOpenGLContextManager::getNumDisplays() const {
    @autoreleasepool {
        return NSScreen.screens.count;
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<MacOSOpenGLContextManager>();
}


END_NAMESPACE_MW
