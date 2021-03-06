//
//  TouchInputDevice.hpp
//  TouchInput
//
//  Created by Christopher Stawarz on 3/20/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef TouchInputDevice_hpp
#define TouchInputDevice_hpp

#import "MWKTouchInputRecognizer.h"


BEGIN_NAMESPACE_MW


class TouchInputDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string TOUCH_POSITION_X;
    static const std::string TOUCH_POSITION_Y;
    static const std::string TOUCH_IN_PROGRESS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TouchInputDevice(const ParameterValueMap &parameters);
    ~TouchInputDevice();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
    void touchBegan(CGPoint location) const;
    void touchMoved(CGPoint newLocation) const;
    void touchEnded() const;
    
private:
    void updatePosition(CGPoint location, MWTime time) const;
    
    const VariablePtr posX;
    const VariablePtr posY;
    const VariablePtr inProgress;
    const boost::shared_ptr<Clock> clock;
    
    GLKMatrix4 projectionMatrix;
    std::array<GLint, 4> viewport;
    
    UIView *targetView;
    MWKTouchInputRecognizer *touchInputRecognizer;
    
    bool started;
    
};


END_NAMESPACE_MW


#endif /* TouchInputDevice_hpp */



























