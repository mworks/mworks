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


bool FirmataChannel::resolvePinNumber(const Datum &requestedPinNumber,
                                      const std::map<std::uint8_t, std::uint8_t> &devicePinForAnalogChannel,
                                      int &pinNumber)
{
    if (requestedPinNumber.isInteger()) {
        pinNumber = requestedPinNumber.getInteger();
        return true;
    }
    
    if (requestedPinNumber.isString()) {
        boost::smatch matchResults;
        if (boost::regex_match(requestedPinNumber.getString(), matchResults, analogChannelIDRegex)) {
            const int number = boost::lexical_cast<int>(matchResults.str("number"));
            const auto iter = devicePinForAnalogChannel.find(number);
            if (iter == devicePinForAnalogChannel.end()) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Analog channel %d does not map to a pin", number);
                return false;
            }
            pinNumber = iter->second;
            return true;
        }
    }
    
    merror(M_IODEVICE_MESSAGE_DOMAIN, "Invalid pin number: %s", requestedPinNumber.toString(true).c_str());
    return false;
}


Datum FirmataChannel::lookupVariable(const std::string &varName) const {
    if (boost::regex_match(varName, analogChannelIDRegex)) {
        return Datum(varName);  // Return analog channel identifier as a string
    }
    if (auto globalVariableRegistry = global_variable_registry) {
        return globalVariableRegistry->lookupVariable(varName);
    }
    return stx::EmptySymbolTable::lookupVariable(varName);  // Always throws
}


Datum FirmataChannel::processFunction(const std::string &funcName, const paramlist_type &paramList) const {
    if (auto globalVariableRegistry = global_variable_registry) {
        return globalVariableRegistry->processFunction(funcName, paramList);
    }
    return stx::EmptySymbolTable::processFunction(funcName, paramList);  // Always throws
}


const boost::regex FirmataChannel::analogChannelIDRegex("[aA](?<number>0|([1-9][0-9]*))");


END_NAMESPACE_MW
