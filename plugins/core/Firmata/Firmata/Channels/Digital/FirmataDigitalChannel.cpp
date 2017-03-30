//
//  FirmataDigitalChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDigitalChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string FirmataDigitalChannel::PIN_NUMBER("pin_number");
const std::string FirmataDigitalChannel::VALUE("value");


void FirmataDigitalChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(PIN_NUMBER);
    info.addParameter(VALUE);
}


FirmataDigitalChannel::FirmataDigitalChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    pinNumber(parameters[PIN_NUMBER]),
    value(parameters[VALUE])
{
    if (pinNumber < 0 || pinNumber > 127) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid pin number");
    }
}


END_NAMESPACE_MW


























