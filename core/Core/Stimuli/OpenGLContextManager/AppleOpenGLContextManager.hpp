//
//  AppleOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#ifndef AppleOpenGLContextManager_hpp
#define AppleOpenGLContextManager_hpp

#include <TargetConditionals.h>
#if TARGET_OS_OSX
#  define GL_SILENCE_DEPRECATION
#  include <Cocoa/Cocoa.h>
#else
#  define GLES_SILENCE_DEPRECATION
#  include <UIKit/UIKit.h>
#  include <OpenGLES/EAGL.h>
#endif

#include "OpenGLContextManager.h"
#include "MWKMetalView.h"


#if TARGET_OS_OSX
@interface MWKOpenGLContext : NSOpenGLContext
#else
@interface MWKOpenGLContext : EAGLContext
#endif

- (mw::OpenGLContextLock)lockContext;

@end


BEGIN_NAMESPACE_MW


class AppleOpenGLContextManager : public OpenGLContextManager {
    
public:
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
    MWKOpenGLContext * getContext(int context_id) const;
    MWKMetalView * getView(int context_id) const;
    
protected:
    NSMutableArray<MWKOpenGLContext *> *contexts;
    NSMutableArray<MWKMetalView *> *views;
#if TARGET_OS_OSX
    NSMutableArray<NSWindow *> *windows;
#else
    NSMutableArray<UIWindow *> *windows;
#endif
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */
