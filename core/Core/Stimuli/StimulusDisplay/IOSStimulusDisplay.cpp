//
//  IOSStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/15/17.
//
//

#include "IOSStimulusDisplay.hpp"

#include "AppleOpenGLContextManager.hpp"
#include "StandardVariables.h"
#include "Utilities.h"


@interface MWKDisplayLinkTarget : NSObject

- (instancetype)initWithStimulusDisplay:(const boost::shared_ptr<mw::IOSStimulusDisplay> &)stimulusDisplay
                           contextIndex:(int)contextIndex
                               callback:(mw::IOSStimulusDisplay::DisplayLinkCallback)callback;

@end


@implementation MWKDisplayLinkTarget {
    boost::weak_ptr<mw::IOSStimulusDisplay> weakStimulusDisplay;
    int contextIndex;
    mw::IOSStimulusDisplay::DisplayLinkCallback callback;
}


- (instancetype)initWithStimulusDisplay:(const boost::shared_ptr<mw::IOSStimulusDisplay> &)stimulusDisplay
                           contextIndex:(int)_contextIndex
                               callback:(mw::IOSStimulusDisplay::DisplayLinkCallback)_callback
{
    self = [super init];
    if (self) {
        weakStimulusDisplay = stimulusDisplay;
        contextIndex = _contextIndex;
        callback = _callback;
    }
    return self;
}


- (void)updateDisplay:(CADisplayLink *)displayLink {
    if (auto stimulusDisplay = weakStimulusDisplay.lock()) {
        try {
            callback(displayLink, *stimulusDisplay, contextIndex);
        } catch (const std::exception &e) {
            mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN, "Error in display link callback: %s", e.what());
        } catch (...) {
            mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN, "Unknown error in display link callback");
        }
    }
}


@end


BEGIN_NAMESPACE_MW


IOSStimulusDisplay::IOSStimulusDisplay(bool announceIndividualStimuli) :
    StimulusDisplay(announceIndividualStimuli),
    displayLinks(nil),
    lastTargetTimestamp(0.0)
{
    @autoreleasepool {
        displayLinks = [[NSMutableArray alloc] init];
    }
}


IOSStimulusDisplay::~IOSStimulusDisplay() {
    @autoreleasepool {
        [displayLinks release];
    }
}


void IOSStimulusDisplay::prepareContext(int contextIndex) {
    @autoreleasepool {
        auto glcm = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(opengl_context_manager);
        auto view = glcm->getView(context_ids.at(contextIndex));
        auto screen = view.window.screen;
        auto sharedThis = boost::dynamic_pointer_cast<IOSStimulusDisplay>(shared_from_this());
        
        MWKDisplayLinkTarget *displayLinkTarget = [[MWKDisplayLinkTarget alloc] initWithStimulusDisplay:sharedThis
                                                                                           contextIndex:contextIndex
                                                                                               callback:&displayLinkCallback];
        CADisplayLink *displayLink = [screen displayLinkWithTarget:displayLinkTarget selector:@selector(updateDisplay:)];
        [displayLinks addObject:displayLink];
        [displayLinkTarget release];
        
        // By default, the display link will invoke the provided selector at the native refresh rate
        // of the display, so there's no need to set its preferredFramesPerSecond property
        NSCAssert(displayLink.preferredFramesPerSecond == 0, @"Unexpected preferredFramesPerSecond on CADisplayLink");
        
        StimulusDisplay::prepareContext(contextIndex);
    }
}


void IOSStimulusDisplay::setMainDisplayRefreshRate() {
    @autoreleasepool {
        // FIXME: maximumFramesPerSecond will be available starting in iOS 10.3.  For now, just assume 60 Hz.
        //auto view = opengl_context_manager->getView(context_ids.at(0));
        //auto screen = view.window.screen;
        //mainDisplayRefreshRate = double(screen.maximumFramesPerSecond);
        mainDisplayRefreshRate = 60.0;
    }
}


void IOSStimulusDisplay::startDisplayUpdates() {
    @autoreleasepool {
        for (CADisplayLink *displayLink in displayLinks) {
            displayLinkThreads.emplace_back([this, displayLink]() {
                @autoreleasepool {
                    NSRunLoop *runLoop = NSRunLoop.currentRunLoop;
                    [displayLink addToRunLoop:runLoop forMode:NSDefaultRunLoopMode];
                    
                    while (displayUpdatesStarted) {
                        @autoreleasepool {
                            [runLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];  // Run for 100ms
                        }
                    }
                    
                    [displayLink removeFromRunLoop:runLoop forMode:NSDefaultRunLoopMode];
                }
            });
        }
    }
}


void IOSStimulusDisplay::stopDisplayUpdates() {
    for (auto &displayLinkThread : displayLinkThreads) {
        displayLinkThread.join();
    }
    displayLinkThreads.clear();
    lastTargetTimestamp = 0.0;
}


void IOSStimulusDisplay::displayLinkCallback(CADisplayLink *displayLink, IOSStimulusDisplay &display, int contextIndex) {
    //
    // This method is only called from display link threads, which manage their own autorelease pools
    //
    
    if (contextIndex != 0) {
        
        display.refreshMirrorDisplay(contextIndex);
        
    } else {
        
        {
            unique_lock lock(display.display_lock);
            
            if (bool(warnOnSkippedRefresh->getValue())) {
                if (display.lastTargetTimestamp) {
                    auto delta = (displayLink.targetTimestamp - display.lastTargetTimestamp) - displayLink.duration;
                    auto numSkippedFrames = std::round(delta / displayLink.duration);
                    if (numSkippedFrames) {
                        mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Skipped %g display refresh cycles", numSkippedFrames);
                    }
                }
            }
            
            display.lastTargetTimestamp = displayLink.targetTimestamp;
            
            //
            // NOTE: This calculation assumes that displayLink.targetTimestamp contains a value returned by
            // CACurrentMediaTime().  According to the docs, values returned by the latter are "derived by
            // calling mach_absolute_time() and converting the result to seconds".
            //
            display.currentOutputTimeUS = (MWTime(displayLink.targetTimestamp * 1e9) -
                                           display.clock->getSystemBaseTimeNS()) / 1000LL;
            
#if 0
            //
            // Check validity of predicted output time
            //
            {
                const auto currentTimeUS = display.clock->getCurrentTimeUS();
                const auto predictedOutputTimeUS = display.currentOutputTimeUS;
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
            
            display.refreshMainDisplay();
            display.waitingForRefresh = false;
        }
        
        // Signal waiting threads that refresh is complete
        display.refreshCond.notify_all();
        
    }
}


boost::shared_ptr<StimulusDisplay> StimulusDisplay::createPlatformStimulusDisplay(bool announceIndividualStimuli) {
    return boost::make_shared<IOSStimulusDisplay>(announceIndividualStimuli);
}


END_NAMESPACE_MW



























