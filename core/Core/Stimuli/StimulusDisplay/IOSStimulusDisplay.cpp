//
//  IOSStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/15/17.
//
//

#include "IOSStimulusDisplay.hpp"

#include "OpenGLContextManager.h"
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
        callback(displayLink, *stimulusDisplay, contextIndex);
    }
}


@end


BEGIN_NAMESPACE_MW


IOSStimulusDisplay::IOSStimulusDisplay(bool announceIndividualStimuli) :
    StimulusDisplay(announceIndividualStimuli),
    displayLinks([[NSMutableArray alloc] init]),
    lastTargetTimestamp(0.0)
{ }


IOSStimulusDisplay::~IOSStimulusDisplay() {
    [displayLinks release];
}


void IOSStimulusDisplay::prepareContext(int contextIndex) {
    auto view = opengl_context_manager->getView(context_ids.at(contextIndex));
    auto screen = view.window.screen;
    auto sharedThis = boost::dynamic_pointer_cast<IOSStimulusDisplay>(shared_from_this());
    
    MWKDisplayLinkTarget *displayLinkTarget = [[MWKDisplayLinkTarget alloc] initWithStimulusDisplay:sharedThis
                                                                                       contextIndex:contextIndex
                                                                                           callback:&displayLinkCallback];
    CADisplayLink *displayLink = [screen displayLinkWithTarget:displayLinkTarget selector:@selector(updateDisplay:)];
    
    [displayLinks addObject:displayLink];
    [displayLink release];
    [displayLinkTarget release];
    
    StimulusDisplay::prepareContext(contextIndex);
}


void IOSStimulusDisplay::setMainDisplayRefreshRate() {
    // FIXME: maximumFramesPerSecond will be available starting in iOS 10.3.  For now, just assume 60 Hz.
    //auto view = opengl_context_manager->getView(context_ids.at(0));
    //auto screen = view.window.screen;
    //mainDisplayRefreshRate = double(screen.maximumFramesPerSecond);
    mainDisplayRefreshRate = 60.0;
}


void IOSStimulusDisplay::startDisplayUpdates() {
    dispatch_sync(dispatch_get_main_queue(), ^{
        for (CADisplayLink *displayLink in displayLinks) {
            // TODO: The main run loop may be too busy to keep up with display refreshes,
            // but we'll try it for now
            [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
        }
    });
}


void IOSStimulusDisplay::stopDisplayUpdates() {
    lastTargetTimestamp = 0.0;
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        for (CADisplayLink *displayLink in displayLinks) {
            [displayLink removeFromRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
        }
    });
}


void IOSStimulusDisplay::displayLinkCallback(CADisplayLink *displayLink, IOSStimulusDisplay &display, int contextIndex) {
    if (contextIndex != 0) {
        
        display.refreshMirrorDisplay(contextIndex);
        
    } else {
        
        {
            unique_lock lock(display.display_lock);
            
            if (bool(warnOnSkippedRefresh->getValue())) {
                if (display.lastTargetTimestamp) {
                    auto delta = (displayLink.targetTimestamp - display.lastTargetTimestamp) - displayLink.duration;
                    auto numSkippedFrames = delta / displayLink.duration;
                    if (delta > 0.1) {
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
            
            display.refreshMainDisplay();
            display.waitingForRefresh = false;
        }
        
        // Signal waiting threads that refresh is complete
        display.refreshCond.notify_all();
        
    }
}


END_NAMESPACE_MW



























