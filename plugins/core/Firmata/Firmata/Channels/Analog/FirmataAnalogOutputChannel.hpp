//
//  FirmataAnalogOutputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/11/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataAnalogOutputChannel_hpp
#define FirmataAnalogOutputChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataAnalogOutputChannel : public FirmataChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataChannel::FirmataChannel;
    
    Type getType() const override { return Type::Analog; }
    Direction getDirection() const override { return Direction::Output; }
    
};


END_NAMESPACE_MW


#endif /* FirmataAnalogOutputChannel_hpp */
