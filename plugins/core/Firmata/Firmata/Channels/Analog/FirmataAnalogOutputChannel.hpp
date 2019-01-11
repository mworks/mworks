//
//  FirmataAnalogOutputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/11/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataAnalogOutputChannel_hpp
#define FirmataAnalogOutputChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataAnalogOutputChannel : public FirmataSimpleChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataSimpleChannel::FirmataSimpleChannel;
    
    Type getType() const override { return Type::Analog; }
    Direction getDirection() const override { return Direction::Output; }
    int getPinMode() const override { return PIN_MODE_PWM; }
    
};


END_NAMESPACE_MW


#endif /* FirmataAnalogOutputChannel_hpp */
