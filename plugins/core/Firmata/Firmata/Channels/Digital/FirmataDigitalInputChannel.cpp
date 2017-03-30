//
//  FirmataDigitalInputChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/17/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDigitalInputChannel.hpp"


BEGIN_NAMESPACE_MW


void FirmataDigitalInputChannel::describeComponent(ComponentInfo &info) {
    FirmataDigitalChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_digital_input");
}


END_NAMESPACE_MW


























