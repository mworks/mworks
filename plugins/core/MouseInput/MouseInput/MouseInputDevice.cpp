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
    info.addParameter(MOUSE_DOWN);
    info.addParameter(HIDE_CURSOR, "NO");
    info.addParameter(USE_MIRROR_WINDOW, "NO");
}


MouseInputDevice::MouseInputDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    posX(parameters[MOUSE_POSITION_X]),
    posY(parameters[MOUSE_POSITION_Y]),
    down(parameters[MOUSE_DOWN]),
    hideCursor(parameters[HIDE_CURSOR]),
    useMirrorWindow(parameters[USE_MIRROR_WINDOW]),
    targetView(nil),
    tracker(nil),
    trackingArea(nil),
    started(false)
{ }


MouseInputDevice::~MouseInputDevice() {
    @autoreleasepool {
        if (trackingArea) {
            [targetView performSelectorOnMainThread:@selector(removeTrackingArea:)
                                         withObject:trackingArea
                                      waitUntilDone:YES];
            [trackingArea release];
        }
        
        if (tracker) {
            if (hideCursor) {
                // Ensure that cursor is unhidden
                [tracker performSelectorOnMainThread:@selector(unhideCursor) withObject:nil waitUntilDone:YES];
            }
            [tracker release];
        }
        
        if (targetView) {
            [targetView release];
        }
    }
}


bool MouseInputDevice::initialize() {
    @autoreleasepool {
        {
            auto glcm = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(OpenGLContextManager::instance());
            if (useMirrorWindow) {
                // If there's no mirror window, getMirrorView will return the fullscreen window's view
                targetView = glcm->getMirrorView();
            } else {
                targetView = glcm->getFullscreenView();
            }
            [targetView retain];
        }
        
        // Get the parameters needed by GLKMathUnproject
        projectionMatrix = StimulusDisplay::getCurrentStimulusDisplay()->getProjectionMatrix();
        {
            [targetView.openGLContext makeCurrentContext];
            OpenGLContextLock ctxLock(targetView.openGLContext.CGLContextObj);
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
        MWTime time = Clock::instance()->getCurrentTimeUS();
        posX->setValue(locationInDegrees.v[0], time);
        posY->setValue(locationInDegrees.v[1], time);
    }
}


void MouseInputDevice::postMouseState(bool isDown) const {
    if (started) {
        down->setValue(isDown, Clock::instance()->getCurrentTimeUS());
    }
}


END_NAMESPACE_MW


























