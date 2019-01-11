//
//  FirmataDigitalInputPullupChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 1/11/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FirmataDigitalInputPullupChannel.hpp"


BEGIN_NAMESPACE_MW


void FirmataDigitalInputPullupChannel::describeComponent(ComponentInfo &info) {
    FirmataSimpleChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_digital_input_pullup");
}


END_NAMESPACE_MW
