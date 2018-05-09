/**
 * MacOSOpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "MacOSOpenGLContextManager.h"

#include "ComponentRegistry.h"
#include "OpenGLUtilities.hpp"


@interface MWKOpenGLView : NSOpenGLView {
    BOOL _opaque;
}

@property(getter=isOpaque) BOOL opaque;

@end


@implementation MWKOpenGLView


- (instancetype)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format
{
    self = [super initWithFrame:frameRect pixelFormat:format];
    if (self) {
        _opaque = YES;
    }
    return self;
}


- (BOOL)isOpaque
{
    return _opaque;
}


- (void)setOpaque:(BOOL)opaque
{
    _opaque = opaque;
}


- (void)update
{
    // This method is called by the windowing system on the main thread.  Since drawing normally occurs
    // on a non-main thread, we need to acquire the context lock before updating the context.
    mw::OpenGLContextLock ctxLock(self.openGLContext.CGLContextObj);
    [super update];
}


@end


BEGIN_NAMESPACE_MW


MacOSOpenGLContextManager::MacOSOpenGLContextManager() :
    display_sleep_block(kIOPMNullAssertionID)
{ }


MacOSOpenGLContextManager::~MacOSOpenGLContextManager() {
    // Calling releaseContexts here causes the application to crash at exit.  Since this class is
    // used as a singleton, it doesn't matter, anyway.
    //releaseContexts();
}


int MacOSOpenGLContextManager::newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) {
    @autoreleasepool {
        if (screen_number < 0 || screen_number >= getNumDisplays()) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Invalid screen number (%d)") % screen_number).str());
        }
        
        NSScreen *screen = NSScreen.screens[screen_number];
        NSRect screen_rect = [screen frame];
        // for some reason, some displays have random values here...
        screen_rect.origin.x = 0.0;
        screen_rect.origin.y = 0.0;
        
        NSOpenGLPixelFormatAttribute attrs[] =
        {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFADoubleBuffer,
            0
        };
        
        NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
        if (!opengl_context) {
            [pixel_format release];
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for fullscreen window");
        }
        
        GLint swap_int = 1;
        [opengl_context setValues: &swap_int forParameter: NSOpenGLCPSwapInterval];
        
        if (!opaque) {
            GLint opacity = 0;
            [opengl_context setValues:&opacity forParameter:NSOpenGLCPSurfaceOpacity];
        }
        
        // Crash on calls to functions removed from the core profile
        CGLEnable(opengl_context.CGLContextObj, kCGLCECrashOnRemovedFunctions);
        
        // NOTE: As of OS X 10.11, performing window and view operations from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            NSWindow *fullscreen_window = [[NSWindow alloc] initWithContentRect:screen_rect
                                                                      styleMask:NSBorderlessWindowMask
                                                                        backing:NSBackingStoreBuffered
                                                                          defer:NO
                                                                         screen:screen];
            
            [fullscreen_window setLevel:NSMainMenuWindowLevel+1];
            
            [fullscreen_window setOpaque:opaque];
            if (!opaque) {
                fullscreen_window.backgroundColor = NSColor.clearColor;
            }
            [fullscreen_window setHidesOnDeactivate:NO];
            
            NSRect view_rect = NSMakeRect(0.0, 0.0, screen_rect.size.width, screen_rect.size.height);
            
            MWKOpenGLView *fullscreen_view = [[MWKOpenGLView alloc] initWithFrame:view_rect pixelFormat:pixel_format];
            if (render_at_full_resolution) {
                fullscreen_view.wantsBestResolutionOpenGLSurface = YES;
            }
            fullscreen_view.opaque = opaque;
            [fullscreen_window setContentView:fullscreen_view];
            [fullscreen_view setOpenGLContext:opengl_context];
            [opengl_context setView:fullscreen_view];
            
            [fullscreen_window makeKeyAndOrderFront:nil];
            
            [windows addObject:fullscreen_window];
            [fullscreen_window release];
            [views addObject:fullscreen_view];
            [fullscreen_view release];
        });
        
        [contexts addObject:opengl_context];
        [opengl_context release];
        [pixel_format release];
        
        if (kIOPMNullAssertionID == display_sleep_block) {
            if (kIOReturnSuccess != IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                                kIOPMAssertionLevelOn,
                                                                (CFStringRef)@"MWorks Prevent Display Sleep",
                                                                &display_sleep_block)) {
                mwarning(M_SERVER_MESSAGE_DOMAIN, "Cannot disable display sleep");
            }
        }
        
        return (contexts.count - 1);
    }
}


int MacOSOpenGLContextManager::newMirrorContext(bool render_at_full_resolution) {
    @autoreleasepool {
        // Determine the width and height of the mirror window
        
        double width = 100.0;    // conspicuously wrong defaults
        double height = 100.0;
        
        shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
        shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
        
        if(main_screen_info != NULL){
            Datum info = *main_screen_info;
            
            if(info.hasKey(M_DISPLAY_WIDTH_KEY)
               && info.hasKey(M_DISPLAY_HEIGHT_KEY)
               && info.hasKey(M_MIRROR_WINDOW_BASE_HEIGHT_KEY)){
                
                double display_width, display_height, display_aspect;
                display_width = info.getElement(M_DISPLAY_WIDTH_KEY);
                display_height = info.getElement(M_DISPLAY_HEIGHT_KEY);
                display_aspect = display_width / display_height;
                height = (double)info.getElement(M_MIRROR_WINDOW_BASE_HEIGHT_KEY);
                width = height * display_aspect;
            }
        }
        
        
        NSOpenGLPixelFormatAttribute attrs[] =
        {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFADoubleBuffer,
            0
        };
        
        NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        NSOpenGLContext *opengl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format
                                                                     shareContext:[getFullscreenView() openGLContext]];
        if (!opengl_context) {
            [pixel_format release];
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL context for mirror window");
        }
        
        GLint swap_int = 1;
        [opengl_context setValues: &swap_int forParameter: NSOpenGLCPSwapInterval];
        
        // Crash on calls to functions removed from the core profile
        CGLEnable(opengl_context.CGLContextObj, kCGLCECrashOnRemovedFunctions);
        
        // NOTE: As of OS X 10.11, performing window and view operations from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            NSRect mirror_rect = NSMakeRect(50.0, 50.0, width, height);
            NSWindow *mirror_window = [[NSWindow alloc] initWithContentRect:mirror_rect
                                                                  styleMask:(NSTitledWindowMask | NSMiniaturizableWindowMask)
                                                                    backing:NSBackingStoreBuffered
                                                                      defer:NO];
            
            NSRect view_rect = NSMakeRect(0.0, 0.0, mirror_rect.size.width, mirror_rect.size.height);
            MWKOpenGLView *mirror_view = [[MWKOpenGLView alloc] initWithFrame:view_rect pixelFormat:pixel_format];
            if (render_at_full_resolution) {
                mirror_view.wantsBestResolutionOpenGLSurface = YES;
            }
            [mirror_window setContentView:mirror_view];
            [mirror_view setOpenGLContext:opengl_context];
            [opengl_context setView:mirror_view];
            
            [mirror_window makeKeyAndOrderFront:nil];
            
            [windows addObject:mirror_window];
            [mirror_window release];
            [views addObject:mirror_view];
            [mirror_view release];
        });
        
        [contexts addObject:opengl_context];
        [opengl_context release];
        [pixel_format release];
        
        return (contexts.count - 1);
    }
}


void MacOSOpenGLContextManager::releaseContexts() {
    @autoreleasepool {
        if (kIOPMNullAssertionID != display_sleep_block) {
            (void)IOPMAssertionRelease(display_sleep_block);  // Ignore the return code
            display_sleep_block = kIOPMNullAssertionID;
        }
        
        [contexts makeObjectsPerformSelector:@selector(clearDrawable)];
        
        // NOTE: As of OS X 10.11, performing window and view operations from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (NSWindow *window in windows) {
                [window orderOut:nil];
            }
            [windows removeAllObjects];
            
            for (NSOpenGLView *view in views) {
                [view clearGLContext];
            }
            [views removeAllObjects];
        });
        
        [contexts removeAllObjects];
    }
}


int MacOSOpenGLContextManager::getNumDisplays() const {
    @autoreleasepool {
        if (auto screens = NSScreen.screens) {
            return screens.count;
        }
        return 0;
    }
}


OpenGLContextLock MacOSOpenGLContextManager::setCurrent(int context_id) {
    @autoreleasepool {
        if (auto context = getContext(context_id)) {
            [context makeCurrentContext];
            return OpenGLContextLock(context.CGLContextObj);
        }
        return OpenGLContextLock();
    }
}


void MacOSOpenGLContextManager::clearCurrent() {
    @autoreleasepool {
        [NSOpenGLContext clearCurrentContext];
    }
}


void MacOSOpenGLContextManager::bindDefaultFramebuffer(int context_id) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void MacOSOpenGLContextManager::flush(int context_id) {
    @autoreleasepool {
        if (auto context = getContext(context_id)) {
            [context flushBuffer];
        }
    }
}


std::vector<float> MacOSOpenGLContextManager::getColorConversionLUTData(int context_id, int numGridPoints) {
    @autoreleasepool {
        std::vector<float> lutData;
        
        if (auto view = getView(context_id)) {
            auto srcProfile = ColorSyncProfilePtr::created(ColorSyncProfileCreateWithName(kColorSyncSRGBProfile));
            
            __block ColorSyncProfilePtr dstProfile;
            dispatch_sync(dispatch_get_main_queue(), ^{
                dstProfile = ColorSyncProfilePtr::borrowed(static_cast<ColorSyncProfileRef>(view.window.colorSpace.colorSyncProfile));
            });
            
            auto transform = createColorSyncTransform(srcProfile, dstProfile);
            getColorConversionLUTData(transform, lutData, numGridPoints);
        }
        
        return lutData;
    }
}


auto MacOSOpenGLContextManager::createColorSyncTransform(const ColorSyncProfilePtr &srcProfile,
                                                         const ColorSyncProfilePtr &dstProfile)
    -> ColorSyncTransformPtr
{
    std::array<CFTypeRef, 3> keys { kColorSyncProfile, kColorSyncRenderingIntent, kColorSyncTransformTag };
    std::array<CFTypeRef, 3> srcVals { srcProfile.get(), kColorSyncRenderingIntentUseProfileHeader, kColorSyncTransformDeviceToPCS };
    std::array<CFTypeRef, 3> dstVals { dstProfile.get(), kColorSyncRenderingIntentUseProfileHeader, kColorSyncTransformPCSToDevice };
    
    auto srcDict = cf::DictionaryPtr::created(CFDictionaryCreate(kCFAllocatorDefault,
                                                                 keys.data(),
                                                                 srcVals.data(),
                                                                 srcVals.size(),
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks));
    
    auto dstDict = cf::DictionaryPtr::created(CFDictionaryCreate(kCFAllocatorDefault,
                                                                 keys.data(),
                                                                 dstVals.data(),
                                                                 dstVals.size(),
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks));
    
    std::array<CFTypeRef, 2>  arrayVals { srcDict.get(), dstDict.get() };
    auto profileSequence = cf::ArrayPtr::created(CFArrayCreate(kCFAllocatorDefault,
                                                               arrayVals.data(),
                                                               arrayVals.size(),
                                                               &kCFTypeArrayCallBacks));
    
    auto transform = ColorSyncTransformPtr::owned(ColorSyncTransformCreate(profileSequence.get(), nullptr));
    if (!transform) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Unable to create ColorSync transform");
    }
    
    return transform;
}


void MacOSOpenGLContextManager::getColorConversionLUTData(const ColorSyncTransformPtr &transform,
                                                          std::vector<float> &lutData,
                                                          int numGridPoints)
{
    auto gridPoints = cf::NumberPtr::created(CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &numGridPoints));
    std::array<CFTypeRef, 1> optKeys { kColorSyncConversionGridPoints };
    std::array<CFTypeRef, 1> optVals { gridPoints.get() };
    auto options = cf::DictionaryPtr::created(CFDictionaryCreate(kCFAllocatorDefault,
                                                                 optKeys.data(),
                                                                 optVals.data(),
                                                                 optVals.size(),
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks));
    
    auto properties = cf::ArrayPtr::owned
    (static_cast<CFArrayRef>(ColorSyncTransformCopyProperty(transform.get(),
                                                            kColorSyncTransformSimplifiedConversionData,
                                                            options.get())));
    cf::DictionaryPtr prop;
    cf::DataPtr data;
    
    if (!properties ||
        CFArrayGetCount(properties.get()) != 1 ||
        !(prop = cf::DictionaryPtr::borrowed(static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(properties.get(), 0)))) ||
        !(data = cf::DataPtr::borrowed(static_cast<CFDataRef>(CFDictionaryGetValue(prop.get(), kColorSyncConversion3DLut)))))
    {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Unable to obtain color conversion lookup table data");
    }
    
    //
    // The data returned by ColorSyncTransformCopyProperty is ordered such that the blue channel varies fastest.
    // However, OpenGL expects red to vary fastest, so we need to reorder the grid points.
    //
    
    constexpr std::size_t numComponents = 3;
    lutData.resize(numGridPoints * numGridPoints * numGridPoints * numComponents);
    
    auto srcPtr = reinterpret_cast<const std::uint16_t *>(CFDataGetBytePtr(data.get()));
    auto dstPtr = lutData.data();
    
    for (int rr = 0; rr < numGridPoints; rr++) {
        for (int gg = 0; gg < numGridPoints; gg++) {
            for (int bb = 0; bb < numGridPoints; bb++) {
                std::ptrdiff_t srcOffset = (rr*numGridPoints*numGridPoints + gg*numGridPoints + bb) * numComponents;
                std::ptrdiff_t dstOffset = (bb*numGridPoints*numGridPoints + gg*numGridPoints + rr) * numComponents;
                for (std::size_t i = 0; i < numComponents; i++) {
                    *(dstPtr + dstOffset + i) = (float(*(srcPtr + srcOffset + i)) /
                                                 float(std::numeric_limits<std::uint16_t>::max()));
                }
            }
        }
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<MacOSOpenGLContextManager>();
}


END_NAMESPACE_MW


























