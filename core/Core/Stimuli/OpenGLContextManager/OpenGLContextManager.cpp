/**
 * OpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "OpenGLContextManager.h"

#if TARGET_OS_OSX
#  include "MacOSOpenGLContextManager.h"
#endif


BEGIN_NAMESPACE_MW


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
#if TARGET_OS_OSX
    return boost::make_shared<MacOSOpenGLContextManager>();
#else
#   error Unsupported platform
#endif
}


SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)


END_NAMESPACE_MW


























