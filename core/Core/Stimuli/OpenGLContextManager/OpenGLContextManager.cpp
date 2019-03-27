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


SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)


END_NAMESPACE_MW
