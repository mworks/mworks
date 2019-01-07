//
//  FirmataAnalogInputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/7/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataAnalogInputChannel_hpp
#define FirmataAnalogInputChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataAnalogInputChannel : public FirmataSimpleChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataSimpleChannel::FirmataSimpleChannel;
    
    Type getType() const override { return Type::Analog; }
    Direction getDirection() const override { return Direction::Input; }
    
};


END_NAMESPACE_MW


#endif /* FirmataAnalogInputChannel_hpp */
