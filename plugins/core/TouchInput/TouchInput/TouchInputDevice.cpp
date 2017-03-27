//
//  TouchInputDevice.cpp
//  TouchInput
//
//  Created by Christopher Stawarz on 3/20/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "TouchInputDevice.hpp"


BEGIN_NAMESPACE_MW


const std::string TouchInputDevice::TOUCH_POSITION_X("touch_position_x");
const std::string TouchInputDevice::TOUCH_POSITION_Y("touch_position_y");
const std::string TouchInputDevice::TOUCH_IN_PROGRESS("touch_in_progress");


void TouchInputDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/touch_input");
    
    info.addParameter(TOUCH_POSITION_X);
    info.addParameter(TOUCH_POSITION_Y);
    info.addParameter(TOUCH_IN_PROGRESS);
}


TouchInputDevice::TouchInputDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    posX(parameters[TOUCH_POSITION_X]),
    posY(parameters[TOUCH_POSITION_Y]),
    inProgress(parameters[TOUCH_IN_PROGRESS]),
    clock(Clock::instance()),
    targetView(nil),
    touchInputRecognizer(nil),
    started(false)
{ }


TouchInputDevice::~TouchInputDevice() {
    @autoreleasepool {
        [touchInputRecognizer release];
        [targetView release];
    }
}


bool TouchInputDevice::initialize() {
    @autoreleasepool {
        auto glcm = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(OpenGLContextManager::instance());
        
        // Get the parameters needed by GLKMathUnproject
        projectionMatrix = StimulusDisplay::getCurrentStimulusDisplay()->getProjectionMatrix();
        {
            auto ctxLock = glcm->setCurrent(0);
            glGetIntegerv(GL_VIEWPORT, viewport.data());
        }
        
        targetView = [glcm->getView(0) retain];
        auto sharedThis = component_shared_from_this<TouchInputDevice>();
        touchInputRecognizer = [[MWKTouchInputRecognizer alloc] initWithTouchInputDevice:sharedThis];
        
        return true;
    }
}


bool TouchInputDevice::startDeviceIO() {
    @autoreleasepool {
        if (!started) {
            [targetView performSelectorOnMainThread:@selector(addGestureRecognizer:)
                                         withObject:touchInputRecognizer
                                      waitUntilDone:YES];
            started = true;
        }
        return true;
    }
}


bool TouchInputDevice::stopDeviceIO() {
    @autoreleasepool {
        if (started) {
            [targetView performSelectorOnMainThread:@selector(removeGestureRecognizer:)
                                         withObject:touchInputRecognizer
                                      waitUntilDone:YES];
            started = false;
        }
        return true;
    }
}


void TouchInputDevice::touchBegan(CGPoint location) const {
    auto time = clock->getCurrentTimeUS();
    updatePosition(location, time);
    inProgress->setValue(true, time);
}


void TouchInputDevice::touchMoved(CGPoint newLocation) const {
    updatePosition(newLocation, clock->getCurrentTimeUS());
}


void TouchInputDevice::touchEnded() const {
    inProgress->setValue(false);
}


void TouchInputDevice::updatePosition(CGPoint location, MWTime time) const {
    //
    // This method is always called from the main thread, so we can access targetView's
    // properties directly, and we don't need to provide an autorelease pool
    //
    
    auto locationInPixels = GLKVector3Make(location.x * targetView.contentScaleFactor,
                                           (targetView.bounds.size.height - location.y) * targetView.contentScaleFactor,
                                           0.0);
    bool success = false;
    auto locationInDegrees = GLKMathUnproject(locationInPixels,
                                              GLKMatrix4Identity,
                                              projectionMatrix,
                                              const_cast<GLint *>(viewport.data()),
                                              &success);
    if (!success) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to convert touch location from window to eye coordinates");
    } else {
        posX->setValue(locationInDegrees.v[0], time);
        posY->setValue(locationInDegrees.v[1], time);
    }
}


END_NAMESPACE_MW



























