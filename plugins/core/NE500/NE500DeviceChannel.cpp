/*
 *  NE500DeviceChannel.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500DeviceChannel.h"


BEGIN_NAMESPACE_MW


const std::string NE500DeviceChannel::CAPABILITY("capability");
const std::string NE500DeviceChannel::VARIABLE("variable");
const std::string NE500DeviceChannel::SYRINGE_DIAMETER("syringe_diameter");
const std::string NE500DeviceChannel::FLOW_RATE("flow_rate");


void NE500DeviceChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    
    info.setSignature("iochannel/ne500");
    
    info.addParameter(CAPABILITY);
    info.addParameter(VARIABLE);
    info.addParameter(SYRINGE_DIAMETER);
    info.addParameter(FLOW_RATE);
}


NE500DeviceChannel::NE500DeviceChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    pump_id(boost::algorithm::to_lower_copy(parameters[CAPABILITY].str())),
    syringe_diameter(parameters[SYRINGE_DIAMETER]),
    rate(parameters[FLOW_RATE]),
    volume(parameters[VARIABLE])
{
    if (pump_id.empty()) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Attempt to access invalid (empty) NE500 pump id");
    }
}


bool NE500DeviceChannel::initialize(const SendFunction &sendMessage) {
    return (sendMessage(pump_id, "DIA " + formatFloat(syringe_diameter)) &&
            sendMessage(pump_id, "FUN RAT") &&
            sendMessage(pump_id, "RAT " + formatFloat(rate->getValue().getFloat()) + " MM"));
}


bool NE500DeviceChannel::dispense(const SendFunction &sendMessage) {
    double amount = volume->getValue().getFloat();
    
    if (amount == 0.0) {
        return true;
    }
    
    string dir_str;
    if(amount >= 0){
        dir_str = "INF"; // infuse
    } else {
        amount *= -1.0;
        dir_str = "WDR"; // withdraw
    }
    
    return (sendMessage(pump_id, "DIR " + dir_str) &&
            sendMessage(pump_id, "RAT " + formatFloat(rate->getValue().getFloat()) + " MM") &&
            sendMessage(pump_id, "VOL " + formatFloat(amount)) &&
            sendMessage(pump_id, "RUN"));
}


std::string NE500DeviceChannel::formatFloat(double val) {
    val = std::max(val, 0.0);
    val = std::min(val, 9999.0);
    
    boost::format fmt;
    
    if (val < 10.0) {
        fmt = boost::format("%.3f");
    } else if (val < 100.0) {
        fmt = boost::format("%.2f");
    } else if (val < 1000.0) {
        fmt = boost::format("%.1f");
    } else {
        fmt = boost::format("%.0f");
    }
    
    return (fmt % val).str();
}


END_NAMESPACE_MW




























