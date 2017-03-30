//
//  FirmataDigitalOutputChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDigitalOutputChannel.hpp"


BEGIN_NAMESPACE_MW


void FirmataDigitalOutputChannel::describeComponent(ComponentInfo &info) {
    FirmataDigitalChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_digital_output");
}


END_NAMESPACE_MW


























