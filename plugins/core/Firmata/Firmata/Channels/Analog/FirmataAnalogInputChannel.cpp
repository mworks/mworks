//
//  FirmataAnalogInputChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/7/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataAnalogInputChannel.hpp"


BEGIN_NAMESPACE_MW


void FirmataAnalogInputChannel::describeComponent(ComponentInfo &info) {
    FirmataChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_analog_input");
}


END_NAMESPACE_MW
