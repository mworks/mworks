/**
 * MacOSStimulusDisplay.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "MacOSStimulusDisplay.h"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


MacOSStimulusDisplay::MacOSStimulusDisplay(const Configuration &config) :
    AppleStimulusDisplay(config),
    displayLink(nil),
    didSetDisplayGamma(false)
{ }


MacOSStimulusDisplay::~MacOSStimulusDisplay() {
    @autoreleasepool {
        if (didSetDisplayGamma) {
            CGDisplayRestoreColorSyncSettings();
        }
        displayLink = nil;
    }
}


void MacOSStimulusDisplay::prepareContext(int context_id, bool isMainContext) {
    AppleStimulusDisplay::prepareContext(context_id, isMainContext);
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        MTKView *view = (isMainContext ? getMainView() : getMirrorView());
        
        if (getUseColorManagement()) {
            // Set the view's color space, so that the system will color match its content
            // to the display’s color space
            view.colorspace = NSColorSpace.sRGBColorSpace.CGColorSpace;
        } else {
            // Despite what the docs say, the color space appears to default to sRGB.  Set
            // it to nil, so that the system does *not* color match the view's content.
            view.colorspace = nil;
        }
        
        if (isMainContext) {
            NSScreen *screen = view.window.screen;
            displayLink = [screen displayLinkWithTarget:getDisplayLinkTarget() selector:@selector(updateDisplay:)];
            
            // By default, the display link will invoke the provided selector at the native refresh rate
            // of the display, so there's no need to set its preferredFramesPerSecond property
            //NSCAssert(displayLink.preferredFramesPerSecond == screen.maximumFramesPerSecond,
            //          @"Unexpected preferredFramesPerSecond on CADisplayLink");
            
            setMainDisplayRefreshRate(double(screen.maximumFramesPerSecond));
            
            if (!getUseColorManagement()) {
                auto &config = getConfiguration();
                if (config.setDisplayGamma) {
                    NSNumber *screenNumber = screen.deviceDescription[@"NSScreenNumber"];
                    CGDirectDisplayID displayID = screenNumber.unsignedIntValue;
                    setDisplayGamma(displayID, config.redGamma, config.greenGamma, config.blueGamma);
                }
            }
        }
    });
}


void MacOSStimulusDisplay::setDisplayGamma(CGDirectDisplayID displayID,
                                           CGGammaValue redGamma,
                                           CGGammaValue greenGamma,
                                           CGGammaValue blueGamma)
{
    if (redGamma <= 0.0 || greenGamma <= 0.0 || blueGamma <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "Invalid display gamma values: red = %g, green = %g, blue = %g",
               redGamma,
               greenGamma,
               blueGamma);
        return;
    }
    
    auto error = CGSetDisplayTransferByFormula(displayID,
                                               0.0, 1.0, 1.0 / redGamma,
                                               0.0, 1.0, 1.0 / greenGamma,
                                               0.0, 1.0, 1.0 / blueGamma);
    if (kCGErrorSuccess != error) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to change display gamma (error = %d)", error);
        return;
    }
    
    didSetDisplayGamma = true;
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::createPlatformStimulusDisplay(const Configuration &config) {
    return boost::make_shared<MacOSStimulusDisplay>(config);
}


END_NAMESPACE_MW
