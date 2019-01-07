//
//  FirmataAnalogOutputChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/11/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataAnalogOutputChannel.hpp"


BEGIN_NAMESPACE_MW


void FirmataAnalogOutputChannel::describeComponent(ComponentInfo &info) {
    FirmataSimpleChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_analog_output");
}


END_NAMESPACE_MW
