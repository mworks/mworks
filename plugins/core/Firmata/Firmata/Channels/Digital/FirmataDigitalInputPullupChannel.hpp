//
//  FirmataDigitalInputPullupChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 1/11/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalInputPullupChannel_hpp
#define FirmataDigitalInputPullupChannel_hpp

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalInputPullupChannel : public FirmataSimpleChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataSimpleChannel::FirmataSimpleChannel;
    
    Type getType() const override { return Type::Digital; }
    Direction getDirection() const override { return Direction::Input; }
    int getPinMode() const override { return PIN_MODE_PULLUP; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalInputPullupChannel_hpp */
