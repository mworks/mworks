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


bool MouseInputDevice::initialize() {
    {
        auto glcm = OpenGLContextManager::instance();
        if (useMirrorWindow) {
            targetView = glcm->getMirrorView();
            if (!targetView) {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Mouse input device: mirror window requested but not found");
                return false;
            }
        } else {
            targetView = glcm->getFullscreenView();
            if (!targetView) {
                targetView = glcm->getMirrorView();
            }
        }
        [targetView retain];
    }
    
    // Get the parameters needed by gluUnProject
    {
        OpenGLContextLock ctxLock = StimulusDisplay::getCurrentStimulusDisplay()->setCurrent(0);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix.data());
        glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix.data());
    }
    {
        OpenGLContextLock ctxLock = OpenGLContextManager::instance()->makeCurrent(targetView.openGLContext);
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


bool MouseInputDevice::startDeviceIO() {
    started = true;
    return true;
}


bool MouseInputDevice::stopDeviceIO() {
    started = false;
    return true;
}


void MouseInputDevice::postMouseLocation(NSPoint location) const {
    if (!started) {
        return;
    }
    
    // This method is always called from the main thread, so we can call convertPointToBacking: directly
    NSPoint locationInPixels = [targetView convertPointToBacking:location];
    GLdouble mouseX, mouseY, mouseZ;
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (GLU_TRUE != gluUnProject(locationInPixels.x, locationInPixels.y, 0.0,
                                 modelViewMatrix.data(),
                                 projectionMatrix.data(),
                                 viewport.data(),
                                 &mouseX, &mouseY, &mouseZ))
#pragma clang diagnostic pop
    {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to convert mouse location from window to eye coordinates");
    } else {
        MWTime time = Clock::instance()->getCurrentTimeUS();
        posX->setValue(mouseX, time);
        posY->setValue(mouseY, time);
    }
}


void MouseInputDevice::postMouseState(bool isDown) const {
    if (started) {
        down->setValue(isDown, Clock::instance()->getCurrentTimeUS());
    }
}


END_NAMESPACE_MW


























