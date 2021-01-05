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

#include <CoreAudio/HostTime.h>

#include "ComponentRegistry.h"
#include "StandardVariables.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


MacOSStimulusDisplay::MacOSStimulusDisplay(bool useColorManagement) :
    AppleStimulusDisplay(useColorManagement),
    displayLink(nil),
    lastFrameTime(0),
    didSetDisplayGamma(false)
{
    std::memset(&currentOutputTimeStamp, 0, sizeof(currentOutputTimeStamp));
}


MacOSStimulusDisplay::~MacOSStimulusDisplay() {
    if (didSetDisplayGamma) {
        CGDisplayRestoreColorSyncSettings();
    }
    CVDisplayLinkRelease(displayLink);
}


void MacOSStimulusDisplay::prepareContext(int context_id, bool isMainContext) {
    AppleStimulusDisplay::prepareContext(context_id, isMainContext);
    
    __block CGDirectDisplayID displayID = kCGNullDirectDisplay;
    dispatch_sync(dispatch_get_main_queue(), ^{
        MTKView *view = (isMainContext ? getMainView() : getMirrorView());
        NSNumber *screenNumber = view.window.screen.deviceDescription[@"NSScreenNumber"];
        displayID = screenNumber.unsignedIntValue;
        if (useColorManagement) {
            // Set the view's color space, so that the system will color match its content
            // to the display’s color space
            view.colorspace = NSColorSpace.sRGBColorSpace.CGColorSpace;
        }
    });
    
    if (!isMainContext) {
        return;
    }
    
    if (kCVReturnSuccess != CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)) {
        throw SimpleException("Unable to create display link");
    }
    
    if (kCVReturnSuccess != CVDisplayLinkSetOutputCallback(displayLink,
                                                           &MacOSStimulusDisplay::displayLinkCallback,
                                                           this))
    {
        throw SimpleException("Unable to set display link output callback");
    }
    
    if (kCVReturnSuccess != CVDisplayLinkSetCurrentCGDisplay(displayLink, displayID)) {
        throw SimpleException("Unable to set current display for display link");
    }
    
    if (!useColorManagement) {
        auto reg = ComponentRegistry::getSharedRegistry();
        auto displayInfo = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME)->getValue();
        if (displayInfo.hasKey(M_SET_DISPLAY_GAMMA_KEY) && displayInfo.getElement(M_SET_DISPLAY_GAMMA_KEY).getBool()) {
            setDisplayGamma(displayInfo, displayID);
        }
    }
    
    {
        CVTime refreshPeriod = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(displayLink);
        double refreshRate = 60.0;
        if (refreshPeriod.flags & kCVTimeIsIndefinite) {
            mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                     "Could not determine main display refresh rate.  Assuming %g Hz.",
                     refreshRate);
        } else {
            refreshRate = double(refreshPeriod.timeScale) / double(refreshPeriod.timeValue);
        }
        mainDisplayRefreshRate = refreshRate;
    }
}


void MacOSStimulusDisplay::setDisplayGamma(const Datum &displayInfo, CGDirectDisplayID displayID) {
    CGGammaValue redGamma = displayInfo.getElement(M_DISPLAY_GAMMA_RED_KEY).getFloat();
    CGGammaValue greenGamma = displayInfo.getElement(M_DISPLAY_GAMMA_GREEN_KEY).getFloat();
    CGGammaValue blueGamma = displayInfo.getElement(M_DISPLAY_GAMMA_BLUE_KEY).getFloat();
    
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


void MacOSStimulusDisplay::startDisplayUpdates() {
    lastFrameTime = 0;
    
    // NOTE: As of OS X 10.11, starting the display link from a non-main thread causes issues
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (kCVReturnSuccess != CVDisplayLinkStart(displayLink)) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Unable to start updates on display %d",
                   CVDisplayLinkGetCurrentCGDisplay(displayLink));
        }
    });
}


void MacOSStimulusDisplay::stopDisplayUpdates() {
    // NOTE: As of OS X 10.11, stopping the display link from a non-main thread causes issues
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (kCVReturnSuccess != CVDisplayLinkStop(displayLink)) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Unable to stop updates on display %d",
                   CVDisplayLinkGetCurrentCGDisplay(displayLink));
        }
    });
    
    std::memset(&currentOutputTimeStamp, 0, sizeof(currentOutputTimeStamp));
}


CVReturn MacOSStimulusDisplay::displayLinkCallback(CVDisplayLinkRef _displayLink,
                                                   const CVTimeStamp *now,
                                                   const CVTimeStamp *outputTime,
                                                   CVOptionFlags flagsIn,
                                                   CVOptionFlags *flagsOut,
                                                   void *_display)
{
    auto &display = *static_cast<MacOSStimulusDisplay *>(_display);
    
    {
        unique_lock lock(display.mutex);
        
        if (display.lastFrameTime) {
            auto delta = (outputTime->videoTime - display.lastFrameTime) - outputTime->videoRefreshPeriod;
            if (delta) {
                display.reportSkippedFrames(double(delta) / double(outputTime->videoRefreshPeriod));
            }
        }
        
        display.lastFrameTime = outputTime->videoTime;
        display.currentOutputTimeStamp = *outputTime;
        
        //
        // Here's how the time calculation works:
        //
        // outputTime->hostTime is the (estimated) time that the frame we're currently drawing will be displayed.
        // The value is in units of the "host time base", which appears to mean that it's directly comparable to
        // the value returned by mach_absolute_time().  However, the relationship to mach_absolute_time() is not
        // explicitly stated in the docs, so presumably we can't depend on it.
        //
        // What the CoreVideo docs *do* say is "the host time base for Core Video and the one for CoreAudio are
        // identical, and the values returned from either API can be used interchangeably".  Hence, we can use the
        // CoreAudio function AudioConvertHostTimeToNanos() to convert from the host time base to nanoseconds.
        //
        // Once we have a system time in nanoseconds, we substract the system base time and convert to
        // microseconds, which leaves us with a value that can be compared to clock->getCurrentTimeUS().
        //
        display.currentOutputTimeUS = (MWTime(AudioConvertHostTimeToNanos(outputTime->hostTime)) -
                                       display.clock->getSystemBaseTimeNS()) / 1000LL;
        
        display.refreshDisplay();
    }
    
    // Signal waiting threads that refresh is complete
    display.refreshCond.notify_all();
    
    return kCVReturnSuccess;
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::createPlatformStimulusDisplay(bool useColorManagement) {
    return boost::make_shared<MacOSStimulusDisplay>(useColorManagement);
}


END_NAMESPACE_MW
