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
#  include <Cocoa/Cocoa.h>
#else
#  include <UIKit/UIKit.h>
#endif

#import <MetalKit/MetalKit.h>

#include "OpenGLContextManager.h"


BEGIN_NAMESPACE_MW


class AppleOpenGLContextManager : public OpenGLContextManager {
    
public:
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
    MTKView * getView(int context_id) const;
    
protected:
    NSMutableArray<MTKView *> *views;
#if TARGET_OS_OSX
    NSMutableArray<NSWindow *> *windows;
#else
    NSMutableArray<UIWindow *> *windows;
#endif
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */
