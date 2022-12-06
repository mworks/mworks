/**
 * OpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "OpenGLContextManager.h"


BEGIN_NAMESPACE_MW


OpenGLContextLock::~OpenGLContextLock() {
    if (lock) {
        OpenGLContextManager::instance()->clearCurrent();
    }
}


int OpenGLContextManager::createFullscreenContext(int screen_number, bool opaque) {
    if (screen_number < 0 || screen_number >= getNumDisplays()) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              (boost::format("Invalid screen number (%d)") % screen_number).str());
    }
    if (screensInUse.contains(screen_number)) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              (boost::format("Screen %d is already in use") % screen_number).str());
    }
    auto contextID = newFullscreenContext(screen_number, opaque);
    screensInUse.insert(screen_number);
    return contextID;
}


int OpenGLContextManager::createMirrorContext(double width, double height, int main_context_id) {
    return newMirrorContext(width, height, main_context_id);
}


void OpenGLContextManager::releaseContexts() {
    screensInUse.clear();
}


SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)


END_NAMESPACE_MW
