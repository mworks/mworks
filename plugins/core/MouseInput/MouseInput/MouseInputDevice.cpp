//
//  MouseInputDevice.cpp
//  MouseInput
//
//  Created by Christopher Stawarz on 5/29/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "MouseInputDevice.h"


BEGIN_NAMESPACE_MW


const std::string MouseInputDevice::MOUSE_POSITION_X("mouse_position_x");
const std::string MouseInputDevice::MOUSE_POSITION_Y("mouse_position_y");
const std::string MouseInputDevice::MOUSE_DOWN("mouse_down");
const std::string MouseInputDevice::HIDE_CURSOR("hide_cursor");
const std::string MouseInputDevice::USE_MIRROR_WINDOW("use_mirror_window");


void MouseInputDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/mouse_input");
    
    info.addParameter(MOUSE_POSITION_X);
    info.addParameter(MOUSE_POSITION_Y);
    info.addParameter(MOUSE_DOWN, false);
    info.addParameter(HIDE_CURSOR, "NO");
    info.addParameter(USE_MIRROR_WINDOW, "NO");
}


MouseInputDevice::MouseInputDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    posX(parameters[MOUSE_POSITION_X]),
    posY(parameters[MOUSE_POSITION_Y]),
    hideCursor(parameters[HIDE_CURSOR]),
    useMirrorWindow(parameters[USE_MIRROR_WINDOW]),
    targetView(nil),
    tracker(nil),
    trackingArea(nil),
    started(false)
{
    if (!(parameters[MOUSE_DOWN].empty())) {
        down = VariablePtr(parameters[MOUSE_DOWN]);
    }
}


MouseInputDevice::~MouseInputDevice() {
    @autoreleasepool {
        if (trackingArea) {
            [targetView performSelectorOnMainThread:@selector(removeTrackingArea:)
                                         withObject:trackingArea
                                      waitUntilDone:YES];
            trackingArea = nil;
        }
        
        if (tracker) {
            if (hideCursor) {
                // Ensure that cursor is unhidden
                [tracker performSelectorOnMainThread:@selector(unhideCursor) withObject:nil waitUntilDone:YES];
            }
            tracker = nil;
        }
        
        targetView = nil;
    }
}


bool MouseInputDevice::initialize() {
    @autoreleasepool {
        {
            // Call getCurrentStimulusDisplay() first, as this will ensure that both the stimulus display and
            // the OpenGL context manager exist
            auto stimulusDisplay = StimulusDisplay::getCurrentStimulusDisplay();
            auto glcm = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(OpenGLContextManager::instance());
            
            // Get the target view and context
            MWKOpenGLContext *targetContext = nil;
            if (useMirrorWindow) {
                // If there's no mirror window, getMirrorView and getMirrorContext will return the fullscreen
                // window's view and context, respectively
                targetView = glcm->getMirrorView();
                targetContext = glcm->getMirrorContext();
            } else {
                targetView = glcm->getFullscreenView();
                targetContext = glcm->getFullscreenContext();
            }
            
            // Get the parameters needed by GLKMathUnproject
            projectionMatrix = stimulusDisplay->getProjectionMatrix();
            [targetContext makeCurrentContext];
            auto ctxLock = [targetContext lockContext];
            glGetIntegerv(GL_VIEWPORT, viewport.data());
        }
        
        tracker = [[MWKMouseTracker alloc] initWithMouseInputDevice:component_shared_from_this<MouseInputDevice>()];
        tracker.shouldHideCursor = hideCursor;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            trackingArea = [[NSTrackingArea alloc] initWithRect:[targetView bounds]
                                                        options:(NSTrackingMouseEnteredAndExited |
                                                                 NSTrackingMouseMoved |
                                                                 NSTrackingActiveAlways)
                                                          owner:tracker
                                                       userInfo:nil];
            
            [targetView addTrackingArea:trackingArea];
            
            if (hideCursor) {
                NSPoint mouseLocationInWindowCoords = targetView.window.mouseLocationOutsideOfEventStream;
                NSPoint mouseLocationInViewCoords = [targetView convertPoint:mouseLocationInWindowCoords fromView:nil];
                if (NSPointInRect(mouseLocationInViewCoords, targetView.bounds)) {
                    // Ensure that the cursor is initially hidden
                    [tracker hideCursor];
                }
            }
        });
        
        return true;
    }
}


bool MouseInputDevice::startDeviceIO() {
    started = true;
    return true;
}


bool MouseInputDevice::stopDeviceIO() {
    started = false;
    return true;
}


void MouseInputDevice::postMouseLocation(NSPoint location) const {
    //
    // This method is always called from the main thread, so we can call targetView's methods directly,
    // and we don't need to provide an autorelease pool
    //
    
    if (!started) {
        return;
    }
    
    NSPoint locationInPixels = [targetView convertPointToBacking:location];
    bool success = false;
    GLKVector3 locationInDegrees = GLKMathUnproject(GLKVector3Make(locationInPixels.x, locationInPixels.y, 0.0),
                                                    GLKMatrix4Identity,
                                                    projectionMatrix,
                                                    const_cast<GLint *>(viewport.data()),
                                                    &success);
    
    if (!success) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to convert mouse location from window to eye coordinates");
    } else {
        updateMousePosition(locationInDegrees.v[0], locationInDegrees.v[1]);
    }
}


void MouseInputDevice::postMouseState(bool isDown) const {
    if (started && down) {
        down->setValue(isDown, Clock::instance()->getCurrentTimeUS());
    }
}


void MouseInputDevice::moveMouseCursor(double xPos, double yPos) const {
    @autoreleasepool {
        GLKVector3 locationInPixels = GLKMathProject(GLKVector3Make(xPos, yPos, 0.0),
                                                     GLKMatrix4Identity,
                                                     projectionMatrix,
                                                     const_cast<GLint *>(viewport.data()));
        __block CGError error = kCGErrorSuccess;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            NSPoint location = [targetView convertPointFromBacking:NSMakePoint(locationInPixels.v[0],
                                                                               locationInPixels.v[1])];
            NSWindow *targetWindow = targetView.window;
            location = [targetWindow convertRectToScreen:NSMakeRect(location.x, location.y, 0, 0)].origin;
            
            // Cocoa screen coordinates have the origin in the lower left, but Quartz screen coordinates put it in the
            // upper left.  Hence, we need to convert location.y before passing it to CGWarpMouseCursorPosition.
            NSScreen *targetScreen = targetWindow.screen;
            NSRect nsFrame = targetScreen.frame;
            CGRect cgFrame = CGDisplayBounds(((NSNumber *)(targetScreen.deviceDescription[@"NSScreenNumber"])).intValue);
            location.y = cgFrame.origin.y + cgFrame.size.height + nsFrame.origin.y - location.y;
            
            error = CGWarpMouseCursorPosition(location);
        });
        
        if (error != kCGErrorSuccess) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to move mouse cursor (error = %d)", error);
        } else {
            // CGWarpMouseCursorPosition doesn't generate a mouse-motion event, so we need to update posX and
            // posY ourselves
            updateMousePosition(xPos, yPos);
        }
    }
}


void MouseInputDevice::updateMousePosition(double x, double y) const {
    MWTime time = Clock::instance()->getCurrentTimeUS();
    posX->setValue(x, time);
    posY->setValue(y, time);
}


END_NAMESPACE_MW


























