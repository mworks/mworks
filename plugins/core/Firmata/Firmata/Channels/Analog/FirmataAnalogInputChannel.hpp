//
//  FirmataAnalogInputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/7/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataAnalogInputChannel_hpp
#define FirmataAnalogInputChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataAnalogInputChannel : public FirmataChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataChannel::FirmataChannel;
    
    Type getType() const override { return Type::Analog; }
    Direction getDirection() const override { return Direction::Input; }
    
};


END_NAMESPACE_MW


#endif /* FirmataAnalogInputChannel_hpp */
