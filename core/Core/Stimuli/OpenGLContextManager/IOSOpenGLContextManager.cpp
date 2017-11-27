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
- (mw::OpenGLContextLock)lockContext;
- (BOOL)prepareGL;
- (void)bindDrawable;
- (void)display;

@end


@implementation MWKEAGLView {
    mw::OpenGLContextLock::unique_lock::mutex_type mutex;
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


- (mw::OpenGLContextLock)lockContext {
    return mw::OpenGLContextLock(mw::OpenGLContextLock::unique_lock(mutex));
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


@interface MWKEAGLViewController : UIViewController
@end


@implementation MWKEAGLViewController


- (BOOL)prefersStatusBarHidden {
    return YES;
}


- (BOOL)shouldAutorotate {
    return NO;
}


@end


BEGIN_NAMESPACE_MW


IOSOpenGLContextManager::~IOSOpenGLContextManager() {
    // Calling releaseContexts here causes the application to crash at exit.  Since this class is
    // used as a singleton, it doesn't matter, anyway.
    //releaseContexts();
}


int IOSOpenGLContextManager::newFullscreenContext(int screen_number, bool render_at_full_resolution) {
    @autoreleasepool {
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
                
                if (MWKEAGLViewController *viewController = [[MWKEAGLViewController alloc] init]) {
                    window.rootViewController = viewController;
                    
                    if (MWKEAGLView *view = [[MWKEAGLView alloc] initWithFrame:window.bounds context:context]) {
                        viewController.view = view;
                        view.contentScaleFactor = (render_at_full_resolution ? screen.scale : 1.0);
                        [EAGLContext setCurrentContext:context];
                        
                        if ([view prepareGL]) {
                            [window makeKeyAndVisible];
                            
                            [contexts addObject:context];
                            [views addObject:view];
                            [windows addObject:window];
                            
                            success = true;
                        }
                        
                        [EAGLContext setCurrentContext:nil];
                        [view release];
                    }
                    
                    [viewController release];
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
}


int IOSOpenGLContextManager::newMirrorContext(bool render_at_full_resolution) {
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror windows are not supported on this OS");
}


void IOSOpenGLContextManager::releaseContexts() {
    @autoreleasepool {
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (UIWindow *window in windows) {
                window.hidden = YES;
            }
            [windows removeAllObjects];
            [views removeAllObjects];
        });
        
        [contexts removeAllObjects];
    }
}


int IOSOpenGLContextManager::getNumDisplays() const {
    // At present, we support only the main display
    return 1;
}


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    @autoreleasepool {
        if (auto view = static_cast<MWKEAGLView *>(getView(context_id))) {
            if ([EAGLContext setCurrentContext:view.context]) {
                auto lock = [view lockContext];
                [view bindDrawable];
                return lock;
            }
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot set current OpenGL ES context");
        }
        return OpenGLContextLock();
    }
}


void IOSOpenGLContextManager::clearCurrent() {
    @autoreleasepool {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Unbind the view's drawable object
        [EAGLContext setCurrentContext:nil];
    }
}


void IOSOpenGLContextManager::bindDefaultFramebuffer(int context_id) {
    @autoreleasepool {
        if (auto view = static_cast<MWKEAGLView *>(getView(context_id))) {
            [view bindDrawable];
        }
    }
}


void IOSOpenGLContextManager::flush(int context_id) {
    @autoreleasepool {
        if (auto view = static_cast<MWKEAGLView *>(getView(context_id))) {
            [view display];
        }
    }
}


std::vector<float> IOSOpenGLContextManager::getColorConversionLUTData(int context_id, int numGridPoints) {
    @autoreleasepool {
        std::vector<float> lutData;
        
        if (auto view = getView(context_id)) {
            __block UIDisplayGamut displayGamut;
            dispatch_sync(dispatch_get_main_queue(), ^{
                displayGamut = view.window.screen.traitCollection.displayGamut;
            });
            
            switch (displayGamut) {
                case UIDisplayGamutSRGB:
                    // No color conversion required
                    break;
                    
                case UIDisplayGamutP3: {
                    //
                    // According to "What's New in Metal, Part 2" (WWDC 2016, Session 605), applications should
                    // always render in the sRGB colorspace, even when the target device has a P3 display.  To use
                    // colors outside of the sRGB gamut, the app needs to use a floating-point color buffer and
                    // encode the P3-only colors using component values less than 0 or greater than 1 (as in Apple's
                    // "extended sRGB" color space).  Since StimulusDisplay uses an 8 bit per channel, integer color
                    // buffer, we're always limited to sRGB.
                    //
                    // Testing suggests that this approach is correct.  If we draw an image with high color
                    // saturation and then display it both with and without a Mac-style, LUT-based conversion
                    // from sRGB to Display P3, the unconverted colors match what we see on a Mac, while the converted
                    // colors are noticeably duller.  This makes sense, because converting, say, 100% red (255, 0, 0)
                    // in sRGB to the wider gamut of Display P3 results in smaller numerical values (234, 51, 35).
                    //
                    // https://developer.apple.com/videos/play/wwdc2016/605/
                    //
                    break;
                }
                    
                default:
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Unsupported display gamut");
            }
            
        }
        
        return lutData;
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<IOSOpenGLContextManager>();
}


END_NAMESPACE_MW


























