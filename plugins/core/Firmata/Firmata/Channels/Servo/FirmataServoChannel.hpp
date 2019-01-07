//
//  FirmataServoChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 11/10/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataServoChannel_hpp
#define FirmataServoChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataServoChannel : public FirmataSimpleChannel {
    
public:
    static const std::string MIN_PULSE_WIDTH;
    static const std::string MAX_PULSE_WIDTH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FirmataServoChannel(const ParameterValueMap &parameters);
    
    Type getType() const override { return Type::Servo; }
    Direction getDirection() const override { return Direction::Output; }
    
    MWTime getMinPulseWidth() const { return minPulseWidth; }
    MWTime getMaxPulseWidth() const { return maxPulseWidth; }
    
private:
    // Min/max pulse widths are transmitted to the device as unsigned 14-bit integers
    static constexpr int minAllowedPulseWidth = 0;
    static constexpr int maxAllowedPulseWidth = 16383;
    
    const MWTime minPulseWidth;
    const MWTime maxPulseWidth;
    
};


END_NAMESPACE_MW


#endif /* FirmataServoChannel_hpp */




















