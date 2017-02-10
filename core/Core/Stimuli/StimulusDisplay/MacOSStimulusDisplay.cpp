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

#include "OpenGLContextManager.h"
#include "StandardVariables.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


MacOSStimulusDisplay::MacOSStimulusDisplay(bool announceIndividualStimuli) :
    StimulusDisplay(announceIndividualStimuli)
{
    std::memset(&currentOutputTimeStamp, 0, sizeof(currentOutputTimeStamp));
}


MacOSStimulusDisplay::~MacOSStimulusDisplay() {
    for (auto dl : displayLinks) {
        CVDisplayLinkRelease(dl);
    }
}


void MacOSStimulusDisplay::setMainDisplayRefreshRate() {
    CVTime refreshPeriod = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(displayLinks.at(0));
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


void MacOSStimulusDisplay::prepareContext(int contextIndex) {
    CVDisplayLinkRef dl;
    
    if (kCVReturnSuccess != CVDisplayLinkCreateWithActiveCGDisplays(&dl)) {
        throw SimpleException("Unable to create display link");
    }
    
    displayLinks.push_back(dl);
    displayLinkContexts.emplace_back(new DisplayLinkContext(this, contextIndex));
    
    if (kCVReturnSuccess != CVDisplayLinkSetOutputCallback(dl,
                                                           &MacOSStimulusDisplay::displayLinkCallback,
                                                           displayLinkContexts.back().get()))
    {
        throw SimpleException("Unable to set display link output callback");
    }
    
    {
        NSOpenGLContext *ctx = opengl_context_manager->getContext(context_ids.at(contextIndex));
        CGLContextObj cglContext = ctx.CGLContextObj;
        CGLPixelFormatObj cglPixelFormat = ((NSOpenGLView *)(ctx.view)).pixelFormat.CGLPixelFormatObj;
        if (kCVReturnSuccess != CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(dl, cglContext, cglPixelFormat)) {
            throw SimpleException("Unable to associate display link with OpenGL context");
        }
    }
    
    StimulusDisplay::prepareContext(contextIndex);
}


void MacOSStimulusDisplay::stateSystemCallback(const Datum &data, MWorksTime time) {
    unique_lock lock(display_lock);
    
    int newState = data.getInteger();
    
    if ((IDLE == newState) && displayUpdatesStarted) {
        
        // If another thread is waiting for a display refresh, allow it to complete before stopping
        // the display link
        while (waitingForRefresh) {
            refreshCond.wait(lock);
        }
        
        displayUpdatesStarted = false;
        std::memset(&currentOutputTimeStamp, 0, sizeof(currentOutputTimeStamp));
        currentOutputTimeUS = -1;
        
        // We need to release the lock before calling CVDisplayLinkStop, because
        // displayLinkCallback could be blocked waiting for the lock, and
        // CVDisplayLinkStop won't return until displayLinkCallback exits, leading to deadlock.
        lock.unlock();
        
        // NOTE: As of OS X 10.11, stopping the display links from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (auto dl : displayLinks) {
                if (kCVReturnSuccess != CVDisplayLinkStop(dl)) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN,
                           "Unable to stop updates on display %d",
                           CVDisplayLinkGetCurrentCGDisplay(dl));
                }
            }
        });
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates stopped");
        
    } else if ((RUNNING == newState) && !displayUpdatesStarted) {
        
        lastFrameTime = 0;
        
        // NOTE: As of OS X 10.11, starting the display links from a non-main thread causes issues
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (auto dl : displayLinks) {
                if (kCVReturnSuccess != CVDisplayLinkStart(dl)) {
                    merror(M_DISPLAY_MESSAGE_DOMAIN,
                           "Unable to start updates on display %d",
                           CVDisplayLinkGetCurrentCGDisplay(dl));
                }
            }
        });
        
        displayUpdatesStarted = true;
        
        // Wait for a refresh to complete, so we know that getCurrentOutputTimeUS() will return a valid time
        ensureRefresh(lock);
        
        mprintf(M_DISPLAY_MESSAGE_DOMAIN, "Display updates started (refresh rate: %g Hz)", getMainDisplayRefreshRate());
        
    }
}


CVReturn MacOSStimulusDisplay::displayLinkCallback(CVDisplayLinkRef _displayLink,
                                              const CVTimeStamp *now,
                                              const CVTimeStamp *outputTime,
                                              CVOptionFlags flagsIn,
                                              CVOptionFlags *flagsOut,
                                              void *_context)
{
    DisplayLinkContext &context = *static_cast<DisplayLinkContext *>(_context);
    MacOSStimulusDisplay &display = *(context.first);
    int contextIndex = context.second;
    
    if (contextIndex != 0) {
        
        display.refreshMirrorDisplay(contextIndex);
        
    } else {
        
        {
            unique_lock lock(display.display_lock);
            
            if (bool(warnOnSkippedRefresh->getValue())) {
                if (display.lastFrameTime) {
                    int64_t delta = (outputTime->videoTime - display.lastFrameTime) - outputTime->videoRefreshPeriod;
                    if (delta) {
                        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                                 "Skipped %g display refresh cycles",
                                 (double)delta / (double)(outputTime->videoRefreshPeriod));
                    }
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
            
            display.refreshMainDisplay();
            display.waitingForRefresh = false;
        }
        
        // Signal waiting threads that refresh is complete
        display.refreshCond.notify_all();
        
    }
    
    return kCVReturnSuccess;
}


END_NAMESPACE_MW


























