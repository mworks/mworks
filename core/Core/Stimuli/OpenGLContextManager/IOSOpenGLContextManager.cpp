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


IOSOpenGLContextManager::IOSOpenGLContextManager() :
    contexts([[NSMutableArray alloc] init]),
    fullscreenView(nil)
{ }


IOSOpenGLContextManager::~IOSOpenGLContextManager() {
    [fullscreenView release];
    [contexts release];
}


int IOSOpenGLContextManager::newFullscreenContext(int index) {
    if (index < 0 || index >= getNMonitors()) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, (boost::format("Invalid screen number (%d)") % index).str());
    }
    
    EAGLSharegroup *sharegroup = nil;
    if (contexts.count > 0) {
        sharegroup = contexts[0].sharegroup;
    }
    
    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];
    if (!context) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL ES context");
    }
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        [fullscreenView release];
        fullscreenView = [[GLKView alloc] initWithFrame:UIScreen.mainScreen.bounds context:context];
    });
    
    if (!fullscreenView) {
        [context release];
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL ES view");
    }
    
    [contexts addObject:context];
    [context release];
    
    return (contexts.count - 1);
}


int IOSOpenGLContextManager::newMirrorContext() {
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror windows are not supported on this OS");
}


void IOSOpenGLContextManager::releaseDisplays() {
    [contexts removeAllObjects];
    
    [fullscreenView release];
    fullscreenView = nil;
    
    mutexes.clear();
}


EAGLContext * IOSOpenGLContextManager::getContext(int context_id) const {
    if (context_id < 0 || context_id >= contexts.count) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
        return nil;
    }
    return contexts[context_id];
}


OpenGLContextLock IOSOpenGLContextManager::makeCurrent(EAGLContext *context) {
    [EAGLContext setCurrentContext:context];
    return OpenGLContextLock(unique_lock(mutexes[context]));
}


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    if (context_id < 0 || context_id >= contexts.count) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
        return OpenGLContextLock();
    }
    return makeCurrent(contexts[context_id]);
}


void IOSOpenGLContextManager::clearCurrent() {
    [EAGLContext setCurrentContext:nil];
}


void IOSOpenGLContextManager::flush(int context_id) {
    // Flushing is performed automatically as part of the drawing loop
}


END_NAMESPACE_MW


























