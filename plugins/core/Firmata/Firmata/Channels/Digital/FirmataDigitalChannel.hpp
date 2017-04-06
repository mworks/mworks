//
//  FirmataDigitalChannel.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDigitalChannel_hpp
#define FirmataDigitalChannel_hpp

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDigitalChannel : public FirmataChannel {
    
public:
    using FirmataChannel::FirmataChannel;
    
    Type getType() const override { return Type::Digital; }
    
};


END_NAMESPACE_MW


#endif /* FirmataDigitalChannel_hpp */



























