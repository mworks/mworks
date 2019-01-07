//
//  FirmataDigitalOutputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalOutputChannel_hpp
#define FirmataDigitalOutputChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalOutputChannel : public FirmataChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataChannel::FirmataChannel;
    
    Type getType() const override { return Type::Digital; }
    Direction getDirection() const override { return Direction::Output; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalOutputChannel_hpp */



























