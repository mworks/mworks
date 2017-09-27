//
//  RCBLVDSDigitalInputChannel.cpp
//  RCBLVDS
//
//  Created by Christopher Stawarz on 9/25/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "RCBLVDSDigitalInputChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string RCBLVDSDigitalInputChannel::CHANNEL("channel");
const std::string RCBLVDSDigitalInputChannel::VALUE("value");


void RCBLVDSDigitalInputChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    
    info.setSignature("iochannel/rcb_lvds_digital_input");
    
    info.addParameter(CHANNEL);
    info.addParameter(VALUE);
}


RCBLVDSDigitalInputChannel::RCBLVDSDigitalInputChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    channelNumbers(getChannelNumbers(parameters[CHANNEL].str())),
    value(parameters[VALUE])
{ }


std::vector<std::size_t> RCBLVDSDigitalInputChannel::getChannelNumbers(const std::string &channel) {
    std::vector<Datum> channelValues;
    ParsedExpressionVariable::evaluateExpressionList(channel, channelValues);
    std::vector<std::size_t> channelNumbers;
    
    for (auto &value : channelValues) {
        auto channelNum = value.getInteger();
        if (channelNum < 0 || channelNum > 15) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  boost::format("Invalid channel number: %d") % channelNum);
        }
        if (std::find(channelNumbers.begin(), channelNumbers.end(), channelNum) != channelNumbers.end()) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  (boost::format("Channel %d appears multiple times in \"%s\" parameter")
                                   % channelNum
                                   % CHANNEL));
        }
        channelNumbers.push_back(channelNum);
    }
    
    return channelNumbers;
}


END_NAMESPACE_MW




















