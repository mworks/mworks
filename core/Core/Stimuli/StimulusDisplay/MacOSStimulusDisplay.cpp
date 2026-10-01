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

#include "MachUtilities.h"
#include "Utilities.h"


@interface MWKDisplayLinkTarget : NSObject

- (instancetype)initWithStimulusDisplay:(const boost::shared_ptr<mw::MacOSStimulusDisplay> &)stimulusDisplay
                               callback:(mw::MacOSStimulusDisplay::DisplayLinkCallback)callback;

@end


@implementation MWKDisplayLinkTarget {
    boost::weak_ptr<mw::MacOSStimulusDisplay> weakStimulusDisplay;
    mw::MacOSStimulusDisplay::DisplayLinkCallback callback;
}


- (instancetype)initWithStimulusDisplay:(const boost::shared_ptr<mw::MacOSStimulusDisplay> &)stimulusDisplay
                               callback:(mw::MacOSStimulusDisplay::DisplayLinkCallback)_callback
{
    self = [super init];
    if (self) {
        weakStimulusDisplay = stimulusDisplay;
        callback = _callback;
    }
    return self;
}


- (void)updateDisplay:(CADisplayLink *)displayLink {
    if (auto stimulusDisplay = weakStimulusDisplay.lock()) {
        try {
            callback(displayLink, *stimulusDisplay);
        } catch (const std::exception &e) {
            mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN, "Error in display link callback: %s", e.what());
        } catch (...) {
            mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN, "Unknown error in display link callback");
        }
    }
}


@end


BEGIN_NAMESPACE_MW


MacOSStimulusDisplay::MacOSStimulusDisplay(const Configuration &config) :
    AppleStimulusDisplay(config),
    displayLink(nil),
    lastTargetTimestamp(0.0),
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
    
    __block CGDirectDisplayID displayID = kCGNullDirectDisplay;
    dispatch_sync(dispatch_get_main_queue(), ^{
        MTKView *view = (isMainContext ? getMainView() : getMirrorView());
        NSScreen *screen = view.window.screen;
        NSNumber *screenNumber = screen.deviceDescription[@"NSScreenNumber"];
        displayID = screenNumber.unsignedIntValue;
        
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
            auto sharedThis = boost::dynamic_pointer_cast<MacOSStimulusDisplay>(shared_from_this());
            MWKDisplayLinkTarget *displayLinkTarget = [[MWKDisplayLinkTarget alloc] initWithStimulusDisplay:sharedThis
                                                                                                   callback:&displayLinkCallback];
            displayLink = [screen displayLinkWithTarget:displayLinkTarget selector:@selector(updateDisplay:)];
            
            // By default, the display link will invoke the provided selector at the native refresh rate
            // of the display, so there's no need to set its preferredFramesPerSecond property
            //NSCAssert(displayLink.preferredFramesPerSecond == screen.maximumFramesPerSecond,
            //          @"Unexpected preferredFramesPerSecond on CADisplayLink");
            
            setMainDisplayRefreshRate(double(screen.maximumFramesPerSecond));
        }
    });
    
    if (isMainContext && !getUseColorManagement()) {
        auto &config = getConfiguration();
        if (config.setDisplayGamma) {
            setDisplayGamma(displayID, config.redGamma, config.greenGamma, config.blueGamma);
        }
    }
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


void MacOSStimulusDisplay::startDisplayUpdates() {
    @autoreleasepool {
        displayLinkThread = std::thread([this]() {
            @autoreleasepool {
                if (!(MachThreadSelf("MWorks CADisplayLink").setPriority(TaskPriority::Stimulus))) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN, "Failed to set priority of display link thread");
                }
                
                NSRunLoop *runLoop = NSRunLoop.currentRunLoop;
                [displayLink addToRunLoop:runLoop forMode:NSDefaultRunLoopMode];
                
                // The display link callback will call CFRunLoopStop when display updates stop,
                // so we can run continuously here
                CFRunLoopRun();
                
                [displayLink removeFromRunLoop:runLoop forMode:NSDefaultRunLoopMode];
            }
        });
    }
}


void MacOSStimulusDisplay::stopDisplayUpdates() {
    displayLinkThread.join();
    lastTargetTimestamp = 0.0;
}


void MacOSStimulusDisplay::displayLinkCallback(CADisplayLink *displayLink, MacOSStimulusDisplay &display) {
    if (!(display.getDisplayUpdatesStarted())) {
        CFRunLoopStop(CFRunLoopGetCurrent());
        return;
    }
    
    @autoreleasepool {
        lock_guard lock(display.mutex);
        
        if (display.lastTargetTimestamp) {
            auto delta = (displayLink.targetTimestamp - display.lastTargetTimestamp) - displayLink.duration;
            auto numSkippedFrames = std::round(delta / displayLink.duration);
            // For some reason, iOS sometimes asks us to draw the same frame twice, which results in a negative
            // number of skipped frames.  This is weird, but it doesn't seem like a situation that we need to
            // report to the user.
            if (numSkippedFrames > 0.0) {
                display.reportSkippedFrames(numSkippedFrames);
            }
        }
        
        display.lastTargetTimestamp = displayLink.targetTimestamp;
        
        //
        // NOTE: This calculation assumes that displayLink.targetTimestamp contains a value returned by
        // CACurrentMediaTime().  According to the docs, values returned by the latter are "derived by
        // calling mach_absolute_time() and converting the result to seconds".
        //
        display.setCurrentOutputTimeUS((MWTime(displayLink.targetTimestamp * 1e9) -
                                        display.getClock()->getSystemBaseTimeNS()) / 1000LL);
        
#if 0
        //
        // Check validity of predicted output time
        //
        {
            const auto currentTimeUS = display.getClock()->getCurrentTimeUS();
            const auto predictedOutputTimeUS = display.getCurrentOutputTimeUS();
            if (predictedOutputTimeUS < currentTimeUS) {
                merror(M_DISPLAY_MESSAGE_DOMAIN,
                       "Predicted display output time is %g ms in the past",
                       (currentTimeUS - predictedOutputTimeUS) / 1000.0);
            } else {
                const auto deltaMS = (predictedOutputTimeUS - currentTimeUS) / 1000.0;
                const auto periodMS = displayLink.duration * 1000.0;
                if (deltaMS > periodMS) {
                    mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                             "Time until predicted display output time (%g ms) "
                             "is greater than display refresh period (%g ms)",
                             deltaMS,
                             periodMS);
                }
            }
        }
#endif
        
        display.refreshDisplay();
    }
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::createPlatformStimulusDisplay(const Configuration &config) {
    return boost::make_shared<MacOSStimulusDisplay>(config);
}


END_NAMESPACE_MW
