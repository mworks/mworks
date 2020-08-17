//
//  FirmataDigitalOutputPulseChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 8/10/20.
//  Copyright © 2020 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalOutputPulseChannel_hpp
#define FirmataDigitalOutputPulseChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalOutputPulseChannel : public FirmataSimpleChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataSimpleChannel::FirmataSimpleChannel;
    
    Type getType() const override { return Type::DigitalPulse; }
    Direction getDirection() const override { return Direction::Output; }
    int getPinMode() const override { return MWORKS_PIN_MODE_OUTPUT_PULSE; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalOutputPulseChannel_hpp */
