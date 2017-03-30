//
//  FirmataDigitalInputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/17/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalInputChannel_hpp
#define FirmataDigitalInputChannel_hpp

#include "FirmataDigitalChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalInputChannel : public FirmataDigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataDigitalChannel::FirmataDigitalChannel;
    
    Direction getDirection() const override { return Direction::Input; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalInputChannel_hpp */



























