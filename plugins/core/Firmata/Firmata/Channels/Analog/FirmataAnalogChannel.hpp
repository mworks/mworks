//
//  FirmataAnalogChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/7/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef FirmataAnalogChannel_hpp
#define FirmataAnalogChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataAnalogChannel : public FirmataChannel {
    
public:
    using FirmataChannel::FirmataChannel;
    
    Type getType() const override { return Type::Analog; }
    
};


END_NAMESPACE_MW


#endif /* FirmataAnalogChannel_hpp */



























