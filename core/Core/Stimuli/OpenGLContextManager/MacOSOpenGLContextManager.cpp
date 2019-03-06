//
//  MacOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/6/19.
//

#include "MacOSOpenGLContextManager.hpp"

#include "ComponentRegistry.h"
#include "OpenGLUtilities.hpp"

#include <Metal/Metal.h>


@interface MWKOpenGLContext : NSOpenGLContext
@end


@implementation MWKOpenGLContext


- (instancetype)init
{
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAAccelerated,
        0
    };
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    if (pixelFormat) {
        self = [super initWithFormat:pixelFormat shareContext:nil];
        if (self) {
            // Crash on calls to functions removed from the core profile
            CGLEnable(self.CGLContextObj, kCGLCECrashOnRemovedFunctions);
        }
        [pixelFormat release];
    }
    return self;
}


@end


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
    return [metalDevice autorelease];
}


int MacOSOpenGLContextManager::newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) {
    @autoreleasepool {
        if (screen_number < 0 || screen_number >= getNumDisplays()) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Invalid screen number (%d)") % screen_number).str());
        }
        
        MWKOpenGLContext *context = [[MWKOpenGLContext alloc] init];
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
                
                if (MWKMetalView *view = [[MWKMetalView alloc] initWithFrame:frame
                                                                      device:getMetalDeviceForScreen(screen)])
                {
                    window.contentView = view;
                    if (!render_at_full_resolution) {
                        view.layer.contentsScale = 1.0;
                        view.drawableSize = view.bounds.size;
                    }
                    view.layer.opaque = opaque;
                    
                    [window makeKeyAndOrderFront:nil];
                    
                    [contexts addObject:context];
                    [views addObject:view];
                    [windows addObject:window];
                    
                    success = true;
                    
                    [view release];
                }
                
                [window release];
            }
        });
        
        [context release];
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create fullscreen window");
        }
        
        if (kIOPMNullAssertionID == display_sleep_block) {
            if (kIOReturnSuccess != IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                                kIOPMAssertionLevelOn,
                                                                (CFStringRef)@"MWorks Prevent Display Sleep",
                                                                &display_sleep_block)) {
                mwarning(M_SERVER_MESSAGE_DOMAIN, "Cannot disable display sleep");
            }
        }
        
        return (contexts.count - 1);
    }
}


int MacOSOpenGLContextManager::newMirrorContext(bool render_at_full_resolution) {
    @autoreleasepool {
        MWKOpenGLContext *context = [[MWKOpenGLContext alloc] init];
        if (!context) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for mirror window");
        }
        
        NSSize size = NSMakeSize(100.0, 100.0);  // Conspicuously wrong defaults
        
        if (auto mainScreenInfo = ComponentRegistry::getSharedRegistry()->getVariable(MAIN_SCREEN_INFO_TAGNAME)) {
            auto value = mainScreenInfo->getValue();
            if (value.hasKey(M_DISPLAY_WIDTH_KEY) &&
                value.hasKey(M_DISPLAY_HEIGHT_KEY) &&
                value.hasKey(M_MIRROR_WINDOW_BASE_HEIGHT_KEY))
            {
                auto displayWidth = value.getElement(M_DISPLAY_WIDTH_KEY).getFloat();
                auto displayHeight = value.getElement(M_DISPLAY_HEIGHT_KEY).getFloat();
                auto displayAspectRatio = displayWidth / displayHeight;
                size.height = value.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY).getFloat();
                size.width = size.height * displayAspectRatio;
            }
        }
        
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            if (NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(50.0, 50.0, size.width, size.height)
                                                               styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable)
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:NO])
            {
                id<MTLDevice> metalDevice = nil;
                if (views.count > 0) {
                    metalDevice = static_cast<MWKMetalView *>(views[0]).device;
                } else {
                    metalDevice = getMetalDeviceForScreen(window.screen);
                }
                
                if (MWKMetalView *view = [[MWKMetalView alloc] initWithFrame:NSMakeRect(0.0, 0.0, size.width, size.height)
                                                                      device:metalDevice])
                {
                    window.contentView = view;
                    if (!render_at_full_resolution) {
                        view.layer.contentsScale = 1.0;
                        view.drawableSize = view.bounds.size;
                    }
                    
                    [window makeKeyAndOrderFront:nil];
                    
                    [contexts addObject:context];
                    [views addObject:view];
                    [windows addObject:window];
                    
                    success = true;
                    
                    [view release];
                }
                
                [window release];
            }
        });
        
        [context release];
        
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
        
        releaseFramebufferTextures();
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (NSWindow *window in windows) {
                [window orderOut:nil];
            }
            [windows removeAllObjects];
            [views removeAllObjects];
        });
        
        [contexts removeAllObjects];
    }
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
            return OpenGLContextLock(context.CGLContextObj);
        }
        return OpenGLContextLock();
    }
}


void MacOSOpenGLContextManager::clearCurrent() {
    @autoreleasepool {
        [NSOpenGLContext clearCurrentContext];
    }
}


void MacOSOpenGLContextManager::prepareContext(int context_id, bool useColorManagement) {
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
                        // Set the view's color space, so that the system will color match its content
                        // to the display’s color space
                        view.colorspace = NSColorSpace.sRGBColorSpace.CGColorSpace;
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
