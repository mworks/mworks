//
//  FirmataChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 4/6/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataChannel.hpp"


BEGIN_NAMESPACE_MW


FirmataChannel::FirmataChannel(const ParameterValueMap &parameters) :
    Component(parameters)
{ }


bool FirmataChannel::resolvePinNumber(const std::string &requestedPinNumber,
                                      const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                      int &pinNumber)
{
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
        
    }
    
    return true;
}


END_NAMESPACE_MW
