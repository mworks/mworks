//
//  FirmataDigitalOutputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalOutputChannel_hpp
#define FirmataDigitalOutputChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalOutputChannel : public FirmataSimpleChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataSimpleChannel::FirmataSimpleChannel;
    
    Type getType() const override { return Type::Digital; }
    Direction getDirection() const override { return Direction::Output; }
    int getPinMode() const override { return PIN_MODE_OUTPUT; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalOutputChannel_hpp */



























