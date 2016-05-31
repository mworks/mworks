//
//  MouseTracker.m
//  MouseInput
//
//  Created by Christopher Stawarz on 5/29/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#import "MouseTracker.h"

#include "MouseInputDevice.h"


@implementation MWKMouseTracker {
    boost::weak_ptr<mw::MouseInputDevice> mouseInputDeviceWeak;
    id upDownEventMonitor;
    id dragEventMonitor;
    BOOL cursorHidden;
}


- (id)initWithMouseInputDevice:(boost::shared_ptr<mw::MouseInputDevice>)mouseInputDevice
{
    if ((self = [super init])) {
        mouseInputDeviceWeak = mouseInputDevice;
        
        upDownEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:(NSLeftMouseDownMask | NSLeftMouseUpMask)
                                                                   handler:^(NSEvent *theEvent) {
                                                                       [self postMouseState:theEvent];
                                                                       return theEvent;
                                                                   }];
        
        dragEventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDraggedMask
                                                                 handler:^(NSEvent *theEvent) {
                                                                     [self postMouseLocation:theEvent];
                                                                     return theEvent;
                                                                 }];
    }
    
    return self;
}


- (void)hideCursor
{
    if (!cursorHidden) {
        // Ensure that the application is in the foreground, so that the cursor will actually hide
        [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
        [NSCursor hide];
        cursorHidden = YES;
    }
}


- (void)unhideCursor
{
    if (cursorHidden) {
        [NSCursor unhide];
        cursorHidden = NO;
    }
}


- (void)mouseEntered:(NSEvent *)theEvent
{
    if (self.shouldHideCursor) {
        [self hideCursor];
    }
    [self postMouseLocation:theEvent];
}


- (void)mouseExited:(NSEvent *)theEvent
{
    if (self.shouldHideCursor) {
        [self unhideCursor];
    }
    [self postMouseLocation:theEvent];
}


- (void)mouseMoved:(NSEvent *)theEvent
{
    [self postMouseLocation:theEvent];
}


- (void)postMouseLocation:(NSEvent *)theEvent
{
    if (auto mouseInputDevice = mouseInputDeviceWeak.lock()) {
        mouseInputDevice->postMouseLocation([theEvent locationInWindow]);
    }
}


- (void)postMouseState:(NSEvent *)theEvent
{
    if (auto mouseInputDevice = mouseInputDeviceWeak.lock()) {
        mouseInputDevice->postMouseState([theEvent type] == NSLeftMouseDown);
    }
}


- (void)dealloc
{
    [NSEvent removeMonitor:upDownEventMonitor];
    [NSEvent removeMonitor:dragEventMonitor];
    [super dealloc];
}


@end

























