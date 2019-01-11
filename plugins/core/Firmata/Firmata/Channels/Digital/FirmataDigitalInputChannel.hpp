//
//  FirmataDigitalInputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/17/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalInputChannel_hpp
#define FirmataDigitalInputChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalInputChannel : public FirmataSimpleChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataSimpleChannel::FirmataSimpleChannel;
    
    Type getType() const override { return Type::Digital; }
    Direction getDirection() const override { return Direction::Input; }
    int getPinMode() const override { return PIN_MODE_INPUT; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalInputChannel_hpp */



























