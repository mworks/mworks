//
//  FirmataAnalogOutputChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/11/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataAnalogOutputChannel_hpp
#define FirmataAnalogOutputChannel_hpp

#include "FirmataAnalogChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataAnalogOutputChannel : public FirmataAnalogChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using FirmataAnalogChannel::FirmataAnalogChannel;
    
    Direction getDirection() const override { return Direction::Output; }
    
};


END_NAMESPACE_MW


#endif /* FirmataAnalogOutputChannel_hpp */
