//
//  MouseInputDevice.h
//  MouseInput
//
//  Created by Christopher Stawarz on 5/29/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#ifndef __MouseInput__MouseInputDevice__
#define __MouseInput__MouseInputDevice__

#import "MouseTracker.h"


BEGIN_NAMESPACE_MW


class MouseInputDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string MOUSE_POSITION_X;
    static const std::string MOUSE_POSITION_Y;
    static const std::string MOUSE_DOWN;
    static const std::string HIDE_CURSOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit MouseInputDevice(const ParameterValueMap &parameters);
    ~MouseInputDevice();
    
    bool initialize() MW_OVERRIDE;
    bool startDeviceIO() MW_OVERRIDE;
    bool stopDeviceIO() MW_OVERRIDE;
    
    void postMouseLocation(NSPoint location) const;
    void postMouseState(bool isDown) const;
    
private:
    VariablePtr posX;
    VariablePtr posY;
    VariablePtr down;
    const bool hideCursor;
    
    std::array<GLdouble, 16> modelViewMatrix;
    std::array<GLdouble, 16> projectionMatrix;
    std::array<GLint, 4> viewport;
    
    NSOpenGLView *mainDisplayView;
    MWKMouseTracker *tracker;
    NSTrackingArea *trackingArea;
    
    bool started;
    
};


END_NAMESPACE_MW


#endif /* !defined(__MouseInput__MouseInputDevice__) */


























