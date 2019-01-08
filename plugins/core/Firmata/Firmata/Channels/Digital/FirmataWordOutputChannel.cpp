//
//  FirmataWordOutputChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 1/8/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FirmataWordOutputChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string FirmataWordOutputChannel::PIN_NUMBERS("pin_numbers");
const std::string FirmataWordOutputChannel::VALUE("value");


void FirmataWordOutputChannel::describeComponent(ComponentInfo &info) {
    FirmataChannel::describeComponent(info);
    
    info.addParameter(PIN_NUMBERS);
    info.addParameter(VALUE);
    
    info.setSignature("iochannel/firmata_word_output");
}


FirmataWordOutputChannel::FirmataWordOutputChannel(const ParameterValueMap &parameters) :
    FirmataChannel(parameters),
    requestedPinNumbers(parameters[PIN_NUMBERS].str()),
    valueVar(parameters[VALUE])
{ }


bool FirmataWordOutputChannel::resolvePinNumbers(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                                 std::vector<int> &pinNumbers)
{
    std::vector<Datum> evaluatedPinNumbers;
    try {
        ParsedExpressionVariable::evaluateExpressionList(requestedPinNumbers, *this, evaluatedPinNumbers);
    } catch (SimpleException &e) {
        merror(e.getDomain(), "%s", e.getMessage().c_str());
        return false;
    }
    
    std::size_t bitNum = 0;
    for (auto &evaluatedPinNumber : evaluatedPinNumbers) {
        int pinNumber = -1;
        if (!resolvePinNumber(evaluatedPinNumber, devicePinForAnalogChannel, pinNumber)) {
            return false;
        }
        if (!(bitForPin.emplace(pinNumber, bitNum).second)) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Word channel pin number list includes pin %d multiple times", pinNumber);
            return false;
        }
        pinNumbers.push_back(pinNumber);
        bitNum++;
    }
    
    return true;
}


Datum FirmataWordOutputChannel::getValueForPin(int pinNumber) {
    const auto iter = bitForPin.find(pinNumber);
    if (iter != bitForPin.end()) {
        return Datum(bool(valueVar->getValue().getInteger() & (1 << iter->second)));
    }
    return Datum(false);
}


void FirmataWordOutputChannel::setValueForPin(int pinNumber, const Datum &value, MWTime time) {
    // This is an output channel, so this method is never called
}


END_NAMESPACE_MW
