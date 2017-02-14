/**
 * MacOSOpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "MacOSOpenGLContextManager.h"

#include "ComponentRegistry.h"


@interface MWKOpenGLView : NSOpenGLView
@end


@implementation MWKOpenGLView


- (void)update
{
    // This method is called by the windowing system on the main thread.  Since drawing normally occurs
    // on a non-main thread, we need to acquire the context lock before updating the context.
    mw::OpenGLContextLock ctxLock(self.openGLContext.CGLContextObj);
    [super update];
}


@end


BEGIN_NAMESPACE_MW


MacOSOpenGLContextManager::MacOSOpenGLContextManager() :
    display_sleep_block(kIOPMNullAssertionID)
{ }


MacOSOpenGLContextManager::~MacOSOpenGLContextManager() {
    releaseContexts();
}


int MacOSOpenGLContextManager::newFullscreenContext(int screen_number) {
    if (screen_number < 0 || screen_number >= getNumDisplays()) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              (boost::format("Invalid screen number (%d)") % screen_number).str());
    }
    
    NSScreen *screen = NSScreen.screens[screen_number];
    NSRect screen_rect = [screen frame];
    // for some reason, some displays have random values here...
    screen_rect.origin.x = 0.0;
    screen_rect.origin.y = 0.0;
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFADoubleBuffer,
        0
    };
    
    NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    if (!opengl_context) {
        [pixel_format release];
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for fullscreen window");
    }
    
    GLint swap_int = 1;
    [opengl_context setValues: &swap_int forParameter: NSOpenGLCPSwapInterval];
    
    // Crash on calls to functions removed from the core profile
    CGLEnable(opengl_context.CGLContextObj, kCGLCECrashOnRemovedFunctions);
    
    // NOTE: As of OS X 10.11, performing window and view operations from a non-main thread causes issues
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSWindow *fullscreen_window = [[NSWindow alloc] initWithContentRect:screen_rect
                                                                  styleMask:NSBorderlessWindowMask
                                                                    backing:NSBackingStoreBuffered
                                                                      defer:NO
                                                                     screen:screen];
        
        [fullscreen_window setLevel:NSMainMenuWindowLevel+1];
        
        [fullscreen_window setOpaque:YES];
        [fullscreen_window setHidesOnDeactivate:NO];
        
        NSRect view_rect = NSMakeRect(0.0, 0.0, screen_rect.size.width, screen_rect.size.height);
        
        MWKOpenGLView *fullscreen_view = [[MWKOpenGLView alloc] initWithFrame:view_rect pixelFormat:pixel_format];
        [fullscreen_window setContentView:fullscreen_view];
        [fullscreen_view setOpenGLContext:opengl_context];
        [opengl_context setView:fullscreen_view];
        
        [fullscreen_window makeKeyAndOrderFront:nil];
        
        [windows addObject:fullscreen_window];
        [fullscreen_window release];
        [views addObject:fullscreen_view];
        [fullscreen_view release];
    });
    
    [contexts addObject:opengl_context];
    [opengl_context release];
    [pixel_format release];
    
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


int MacOSOpenGLContextManager::newMirrorContext(){
    
    // Determine the width and height of the mirror window

    double width = 100.0;    // conspicuously wrong defaults
	double height = 100.0;
	
    shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
    shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
    
	if(main_screen_info != NULL){
        Datum info = *main_screen_info;
		
		if(info.hasKey(M_DISPLAY_WIDTH_KEY) 
		   && info.hasKey(M_DISPLAY_HEIGHT_KEY)
		   && info.hasKey(M_MIRROR_WINDOW_BASE_HEIGHT_KEY)){
			
			double display_width, display_height, display_aspect;
			display_width = info.getElement(M_DISPLAY_WIDTH_KEY);
			display_height = info.getElement(M_DISPLAY_HEIGHT_KEY);
			display_aspect = display_width / display_height;
			height = (double)info.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY);
			width = height * display_aspect;
		}
	}
    
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFADoubleBuffer,
        0
    };
    
    NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format
                                                                 shareContext:[getFullscreenView() openGLContext]];
    if (!opengl_context) {
        [pixel_format release];
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for mirror window");
    }
    
    GLint swap_int = 1;
    [opengl_context setValues: &swap_int forParameter: NSOpenGLCPSwapInterval];
    
    // Crash on calls to functions removed from the core profile
    CGLEnable(opengl_context.CGLContextObj, kCGLCECrashOnRemovedFunctions);
    
    // NOTE: As of OS X 10.11, performing window and view operations from a non-main thread causes issues
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSRect mirror_rect = NSMakeRect(50.0, 50.0, width, height);
        NSWindow *mirror_window = [[NSWindow alloc] initWithContentRect:mirror_rect
                                                              styleMask:(NSTitledWindowMask | NSMiniaturizableWindowMask)
                                                                backing:NSBackingStoreBuffered
                                                                  defer:NO];
        
        NSRect view_rect = NSMakeRect(0.0, 0.0, mirror_rect.size.width, mirror_rect.size.height);
        MWKOpenGLView *mirror_view = [[MWKOpenGLView alloc] initWithFrame:view_rect pixelFormat:pixel_format];
        [mirror_window setContentView:mirror_view];
        [mirror_view setOpenGLContext:opengl_context];
        [opengl_context setView:mirror_view];
        
        [mirror_window makeKeyAndOrderFront:nil];
        
        [windows addObject:mirror_window];
        [mirror_window release];
        [views addObject:mirror_view];
        [mirror_view release];
    });
    
    [contexts addObject:opengl_context];
    [opengl_context release];
    [pixel_format release];
    
    return (contexts.count - 1);
}


void MacOSOpenGLContextManager::releaseContexts() {
    if (kIOPMNullAssertionID != display_sleep_block) {
        (void)IOPMAssertionRelease(display_sleep_block);  // Ignore the return code
        display_sleep_block = kIOPMNullAssertionID;
    }
    
    [contexts makeObjectsPerformSelector:@selector(clearDrawable)];
    
    // NOTE: As of OS X 10.11, performing window and view operations from a non-main thread causes issues
    dispatch_sync(dispatch_get_main_queue(), ^{
        for (NSWindow *window : windows) {
            [window orderOut:nil];
        }
        [windows removeAllObjects];
        
        for (NSOpenGLView *view : views) {
            [view clearGLContext];
        }
        [views removeAllObjects];
    });
    
    [contexts removeAllObjects];
}


int MacOSOpenGLContextManager::getNumDisplays() const {
    if (auto screens = NSScreen.screens) {
        return screens.count;
    }
    return 0;
}


OpenGLContextLock MacOSOpenGLContextManager::makeCurrent(NSOpenGLContext *context) {
    if (context) {
        [context makeCurrentContext];
        return OpenGLContextLock(context.CGLContextObj);
    }
    return OpenGLContextLock();
}


OpenGLContextLock MacOSOpenGLContextManager::setCurrent(int context_id) {
    return makeCurrent(getContext(context_id));
}


void MacOSOpenGLContextManager::clearCurrent() {
    [NSOpenGLContext clearCurrentContext];
}


void MacOSOpenGLContextManager::flush(int context_id) {
    if (auto ctx = getContext(context_id)) {
        [ctx flushBuffer];
    }
}


END_NAMESPACE_MW


























