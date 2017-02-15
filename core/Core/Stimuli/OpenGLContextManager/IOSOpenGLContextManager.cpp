//
//  IOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#include "IOSOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"


//
// The technique for using a CAEAGLLayer-backed UIView for displaying OpenGL ES content is taken from
// Apple's "Real-time Video Processing Using AVPlayerItemVideoOutput" example project:
//
// https://developer.apple.com/library/content/samplecode/AVBasicVideoOutput/Introduction/Intro.html
//


@interface MWKEAGLView : UIView

@property(nonatomic, readonly) EAGLContext *context;

- (instancetype)initWithFrame:(CGRect)frame context:(EAGLContext *)context;
- (BOOL)prepareGL;
- (void)bindDrawable;
- (void)display;

@end


@implementation MWKEAGLView {
    GLuint framebuffer;
    GLuint renderbuffer;
}


+ (Class)layerClass {
    return [CAEAGLLayer class];
}


- (instancetype)initWithFrame:(CGRect)frame context:(EAGLContext *)context {
    self = [super initWithFrame:frame];
    
    if (self) {
        _context = [context retain];
        self.layer.opaque = TRUE;
    }
    
    return self;
}


- (BOOL)prepareGL {
    glGenFramebuffers(1, &framebuffer);
    mw::gl::FramebufferBinding<GL_FRAMEBUFFER> framebufferBinding(framebuffer);
    
    glGenRenderbuffers(1, &renderbuffer);
    mw::gl::RenderbufferBinding<GL_RENDERBUFFER> renderbufferBinding(renderbuffer);
    
    [self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:static_cast<CAEAGLLayer *>(self.layer)];
    
    GLint backingWidth, backingHeight;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
    glViewport(0, 0, backingWidth, backingHeight);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    return (GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
}


- (void)bindDrawable {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}


- (void)display {
    mw::gl::RenderbufferBinding<GL_RENDERBUFFER> renderbufferBinding(renderbuffer);
    [self.context presentRenderbuffer:GL_RENDERBUFFER];
}


- (void)dealloc {
    [_context release];
    [super dealloc];
}


@end


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
            
            if (MWKEAGLView *view = [[MWKEAGLView alloc] initWithFrame:window.bounds context:context]) {
                [EAGLContext setCurrentContext:context];
                
                if ([view prepareGL]) {
                    [window addSubview:view];
                    [window makeKeyAndVisible];
                    
                    [contexts addObject:context];
                    [views addObject:view];
                    [windows addObject:window];
                    
                    success = true;
                }
                
                [EAGLContext setCurrentContext:nil];
                
                [view release];
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
        for (UIWindow *window in windows) {
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


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    if (auto view = static_cast<MWKEAGLView *>(getView(context_id))) {
        if ([EAGLContext setCurrentContext:view.context]) {
            [view bindDrawable];
            return OpenGLContextLock(unique_lock(mutexes[view.context]));
        }
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot set current OpenGL ES context");
    }
    return OpenGLContextLock();
}


void IOSOpenGLContextManager::clearCurrent() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Unbind the view's drawable object
    [EAGLContext setCurrentContext:nil];
}


void IOSOpenGLContextManager::bindDefaultFramebuffer(int context_id) {
    if (auto view = static_cast<MWKEAGLView *>(getView(context_id))) {
        [view bindDrawable];
    }
}


void IOSOpenGLContextManager::flush(int context_id) {
    if (auto view = static_cast<MWKEAGLView *>(getView(context_id))) {
        [view display];
    }
}


END_NAMESPACE_MW


























