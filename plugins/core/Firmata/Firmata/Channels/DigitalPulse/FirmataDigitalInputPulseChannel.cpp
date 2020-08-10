//
//  FirmataDigitalInputPulseChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 8/10/20.
//  Copyright © 2020 The MWorks Project. All rights reserved.
//

#include "FirmataDigitalInputPulseChannel.hpp"


BEGIN_NAMESPACE_MW


void FirmataDigitalInputPulseChannel::describeComponent(ComponentInfo &info) {
    FirmataSimpleChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_digital_input_pulse");
}


END_NAMESPACE_MW
