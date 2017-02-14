//
//  IOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#include "IOSOpenGLContextManager.hpp"

#import <UIKit/UIKit.h>


BEGIN_NAMESPACE_MW


IOSOpenGLContextManager::~IOSOpenGLContextManager() {
    releaseContexts();
}


int IOSOpenGLContextManager::newFullscreenContext(int screen_number) {
    if (screen_number < 0 || screen_number >= getNumDisplays()) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              (boost::format("Invalid screen number (%d)") % screen_number).str());
    }
    
    EAGLSharegroup *sharegroup = nil;
    if (contexts.count > 0) {
        sharegroup = static_cast<EAGLContext *>(contexts[0]).sharegroup;
    }
    
    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];
    if (!context) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL ES context");
    }
    
    auto screen = UIScreen.screens[screen_number];
    __block bool success = false;
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (UIWindow *window = [[UIWindow alloc] initWithFrame:screen.bounds]) {
            window.screen = screen;
            
            if (GLKView *view = [[GLKView alloc] initWithFrame:window.bounds context:context]) {
                [window addSubview:view];
                [window makeKeyAndVisible];
                
                [contexts addObject:context];
                [views addObject:view];
                [windows addObject:window];
                
                [view release];
                success = true;
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


int IOSOpenGLContextManager::newMirrorContext() {
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror windows are not supported on this OS");
}


void IOSOpenGLContextManager::releaseContexts() {
    mutexes.clear();
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        for (UIWindow *window : windows) {
            window.hidden = YES;
        }
        [windows removeAllObjects];
        [views removeAllObjects];
    });
    
    [contexts removeAllObjects];
}


int IOSOpenGLContextManager::getNumDisplays() const {
    // At present, we support only the main display
    return 1;
}


OpenGLContextLock IOSOpenGLContextManager::makeCurrent(EAGLContext *context) {
    if (context) {
        [EAGLContext setCurrentContext:context];
        return OpenGLContextLock(unique_lock(mutexes[context]));
    }
    return OpenGLContextLock();
}


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    return makeCurrent(getContext(context_id));
}


void IOSOpenGLContextManager::clearCurrent() {
    [EAGLContext setCurrentContext:nil];
}


void IOSOpenGLContextManager::flush(int context_id) {
    // Flushing is performed automatically as part of the drawing loop
}


END_NAMESPACE_MW


























