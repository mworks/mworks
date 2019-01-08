//
//  FirmataSimpleChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 1/7/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FirmataSimpleChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string FirmataSimpleChannel::PIN_NUMBER("pin_number");
const std::string FirmataSimpleChannel::VALUE("value");


void FirmataSimpleChannel::describeComponent(ComponentInfo &info) {
    FirmataChannel::describeComponent(info);
    info.addParameter(PIN_NUMBER);
    info.addParameter(VALUE);
}


FirmataSimpleChannel::FirmataSimpleChannel(const ParameterValueMap &parameters) :
    FirmataChannel(parameters),
    requestedPinNumber(parameters[PIN_NUMBER].str()),
    valueVar(parameters[VALUE])
{ }


bool FirmataSimpleChannel::resolvePinNumbers(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                             std::vector<int> &pinNumbers)
{
    Datum evaluatedPinNumber;
    try {
        evaluatedPinNumber = ParsedExpressionVariable::evaluateExpression(requestedPinNumber, *this);
    } catch (SimpleException &e) {
        merror(e.getDomain(), "%s", e.getMessage().c_str());
        return false;
    }
    
    int pinNumber = -1;
    if (!resolvePinNumber(evaluatedPinNumber, devicePinForAnalogChannel, pinNumber)) {
        return false;
    }
    
    pinNumbers.push_back(pinNumber);
    return true;
}


END_NAMESPACE_MW
