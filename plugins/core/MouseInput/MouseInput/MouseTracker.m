//
//  MouseTracker.m
//  MouseInput
//
//  Created by Christopher Stawarz on 5/29/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#import "MouseTracker.h"

#include "MouseInputDevice.h"


@implementation MWKMouseTracker


@synthesize shouldHideCursor;


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


- (void)mouseEntered:(NSEvent *)theEvent
{
    // NOTE: The cursor isn't hidden until a mouse down event occurs inside the tracking area.  This isn't an
    // issue for touchscreens, since, in that case, the cursor position changes *only* on mouse down events.
    // However, it can be confusing when you're trying to test cursor hiding with a regular mouse.
    if (self.shouldHideCursor) {
        [NSCursor hide];
    }
    [self postMouseLocation:theEvent];
}


- (void)mouseExited:(NSEvent *)theEvent
{
    if (self.shouldHideCursor) {
        [NSCursor unhide];
    }
    [self postMouseLocation:theEvent];
}


- (void)mouseMoved:(NSEvent *)theEvent
{
    [self postMouseLocation:theEvent];
}


- (void)postMouseLocation:(NSEvent *)theEvent
{
    boost::shared_ptr<mw::MouseInputDevice> mouseInputDevice = mouseInputDeviceWeak.lock();
    if (mouseInputDevice) {
        mouseInputDevice->postMouseLocation([theEvent locationInWindow]);
    }
}


- (void)postMouseState:(NSEvent *)theEvent
{
    boost::shared_ptr<mw::MouseInputDevice> mouseInputDevice = mouseInputDeviceWeak.lock();
    if (mouseInputDevice) {
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

























