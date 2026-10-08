//
//  IOSStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/15/17.
//
//

#define GLES_SILENCE_DEPRECATION

#include "IOSStimulusDisplay.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


IOSStimulusDisplay::IOSStimulusDisplay(const Configuration &config) :
    AppleStimulusDisplay(config),
    displayLink(nil)
{ }


IOSStimulusDisplay::~IOSStimulusDisplay() {
    @autoreleasepool {
        displayLink = nil;
    }
}


void IOSStimulusDisplay::prepareContext(int context_id, bool isMainContext) {
    AppleStimulusDisplay::prepareContext(context_id, isMainContext);
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        MTKView *view = (isMainContext ? getMainView() : getMirrorView());
        UIScreen *screen = view.window.screen;
        
        if (getUseColorManagement()) {
            UIDisplayGamut displayGamut = screen.traitCollection.displayGamut;
            switch (displayGamut) {
                case UIDisplayGamutSRGB:
                    // No color conversion required
                    break;
                    
                case UIDisplayGamutP3:
                    //
                    // According to "What's New in Metal, Part 2" (WWDC 2016, Session 605), applications should
                    // always render in the sRGB colorspace, even when the target device has a P3 display.  To use
                    // colors outside of the sRGB gamut, the app needs to use a floating-point color buffer and
                    // encode the P3-only colors using component values less than 0 or greater than 1 (as in Apple's
                    // "extended sRGB" color space).  Since MTKView uses an 8 bit per channel, integer color
                    // buffer by default, we're always limited to sRGB.
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
                    
                default:
                    mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Unknown display gamut (%ld)", displayGamut);
                    break;
            }
        }
        
        if (isMainContext) {
            displayLink = [screen displayLinkWithTarget:getDisplayLinkTarget() selector:@selector(updateDisplay:)];
            
            // By default, the display link will invoke the provided selector at the native refresh rate
            // of the display, so there's no need to set its preferredFramesPerSecond property
            //NSCAssert(displayLink.preferredFramesPerSecond == screen.maximumFramesPerSecond,
            //          @"Unexpected preferredFramesPerSecond on CADisplayLink");
            
            setMainDisplayRefreshRate(double(screen.maximumFramesPerSecond));
        }
    });
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::createPlatformStimulusDisplay(const Configuration &config) {
    return boost::make_shared<IOSStimulusDisplay>(config);
}


END_NAMESPACE_MW
