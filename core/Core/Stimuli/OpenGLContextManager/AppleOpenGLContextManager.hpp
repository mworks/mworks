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
#  define MWORKS_HAVE_OPENGL 1
#  include <Cocoa/Cocoa.h>
#else
#  include <UIKit/UIKit.h>
#endif

#import <MetalKit/MetalKit.h>

#include "OpenGLContextManager.h"


#if MWORKS_HAVE_OPENGL
@interface MWKOpenGLContext : NSOpenGLContext

- (mw::OpenGLContextLock)lockContext;

@end
#endif


BEGIN_NAMESPACE_MW


class AppleOpenGLContextManager : public OpenGLContextManager {
    
public:
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
#if MWORKS_HAVE_OPENGL
    MWKOpenGLContext * getContext(int context_id) const;
#endif
    MTKView * getView(int context_id) const;
    
protected:
#if MWORKS_HAVE_OPENGL
    NSMutableArray<MWKOpenGLContext *> *contexts;
#endif
    NSMutableArray<MTKView *> *views;
#if TARGET_OS_OSX
    NSMutableArray<NSWindow *> *windows;
#else
    NSMutableArray<UIWindow *> *windows;
#endif
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */
