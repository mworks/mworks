//
//  FirmataChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/6/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string FirmataChannel::PIN_NUMBER("pin_number");
const std::string FirmataChannel::VALUE("value");


void FirmataChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(PIN_NUMBER);
    info.addParameter(VALUE);
}


FirmataChannel::FirmataChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    requestedPinNumber(parameters[PIN_NUMBER].str()),
    value(parameters[VALUE]),
    pinNumber(-1),
    analogChannelNumber(-1)
{ }


bool FirmataChannel::resolvePinNumber(const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel) {
    static const boost::regex pinNumberRegex("(?<analog_channel_prefix>[aA])?(?<number>0|([1-9][0-9]*))");
    boost::smatch matchResults;
    
    if (!boost::regex_match(requestedPinNumber, matchResults, pinNumberRegex)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Invalid pin number: \"%s\"", requestedPinNumber.c_str());
        return false;
    }
    
    const int number = boost::lexical_cast<int>(matchResults.str("number"));
    
    if (!(matchResults["analog_channel_prefix"].matched)) {
        
        if (number < minPinNumber || number > maxPinNumber) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Pin number must be between %d and %d; %d is not valid",
                   minPinNumber,
                   maxPinNumber,
                   number);
            return false;
        }
        
        pinNumber = number;
        
        for (auto &item : devicePinForAnalogChannel) {
            if (item.second == number) {
                analogChannelNumber = item.first;
                break;
            }
        }
        
    } else {
        
        if (number < minAnalogChannelNumber || number > maxAnalogChannelNumber) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Analog channel number must be between %d and %d; %d is not valid",
                   minAnalogChannelNumber,
                   maxAnalogChannelNumber,
                   number);
            return false;
        }
        
        const auto iter = devicePinForAnalogChannel.find(number);
        if (iter == devicePinForAnalogChannel.end()) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Analog channel %d does not map to a pin", number);
            return false;
        }
        
        pinNumber = iter->second;
        analogChannelNumber = number;
        
    }
    
    return true;
}


END_NAMESPACE_MW


























