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
const std::string NE500DeviceChannel::SYRINGE_DIAMETER("syringe_diameter");
const std::string NE500DeviceChannel::FLOW_RATE("flow_rate");
const std::string NE500DeviceChannel::RATE_UNIT("rate_unit");
const std::string NE500DeviceChannel::VARIABLE("variable");
const std::string NE500DeviceChannel::VOLUME_UNIT("volume_unit");


void NE500DeviceChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    
    info.setSignature("iochannel/ne500");
    
    info.addParameter(CAPABILITY);
    info.addParameter(SYRINGE_DIAMETER);
    info.addParameter(FLOW_RATE);
    info.addParameter(RATE_UNIT, "MM");
    info.addParameter(VARIABLE);
    info.addParameter(VOLUME_UNIT, false);
}


static inline std::string getRateUnit(std::string unit) {
    boost::algorithm::trim(unit);
    boost::algorithm::to_upper(unit);
    
    if (unit != "UM" &&
        unit != "MM" &&
        unit != "UH" &&
        unit != "MH")
    {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid NE500 rate unit", unit);
    }
    
    return std::move(unit);
}


static inline std::string getVolumeUnit(std::string unit, double syringeDiameter) {
    boost::algorithm::trim(unit);
    
    if (unit.empty()) {
        if (syringeDiameter <= 14.0) {
            unit = "UL";
        } else {
            unit = "ML";
        }
    } else {
        boost::algorithm::to_upper(unit);
        
        if (unit != "UL" &&
            unit != "ML")
        {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid NE500 volume unit", unit);
        }
    }
    
    return std::move(unit);
}


NE500DeviceChannel::NE500DeviceChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    pump_id(boost::algorithm::to_lower_copy(parameters[CAPABILITY].str())),
    syringe_diameter(parameters[SYRINGE_DIAMETER]),
    rate(parameters[FLOW_RATE]),
    rateUnit(getRateUnit(parameters[RATE_UNIT].str())),
    volume(parameters[VARIABLE]),
    volumeUnit(getVolumeUnit(parameters[VOLUME_UNIT].str(), syringe_diameter)),
    previousRate(0.0),
    previousAbsVolume(0.0),
    previousDirection(Direction::Infuse)
{
    if (pump_id.empty()) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Attempt to access invalid (empty) NE500 pump id");
    }
}


bool NE500DeviceChannel::initialize(const SendFunction &sendMessage) {
    const double currentRate = getCurrentRate();
    const double currentVolume = getCurrentVolume();
    const Direction currentDirection = getCurrentDirection(currentVolume);
    
    return (sendMessage(pump_id, "DIA " + formatFloat(syringe_diameter)) &&
            sendMessage(pump_id, "PHN 2") &&
            sendMessage(pump_id, "FUN STP") &&
            sendMessage(pump_id, "PHN 1") &&
            sendMessage(pump_id, "FUN RAT") &&
            setRate(sendMessage, currentRate) &&
            sendMessage(pump_id, "VOL " + volumeUnit) &&
            setVolume(sendMessage, currentVolume) &&
            setDirection(sendMessage, currentDirection));
}


bool NE500DeviceChannel::dispense(const SendFunction &sendMessage) {
    const double currentRate = getCurrentRate();
    const double currentVolume = getCurrentVolume();
    const Direction currentDirection = getCurrentDirection(currentVolume);
    
    if (currentVolume == 0.0) {
        return true;
    }
    
    if (currentRate != previousRate) {
        if (!setRate(sendMessage, currentRate)) {
            return false;
        }
    }
    
    if (std::abs(currentVolume) != previousAbsVolume) {
        if (!setVolume(sendMessage, currentVolume)) {
            return false;
        }
    }
    
    if (currentDirection != previousDirection) {
        if (!setDirection(sendMessage, currentDirection)) {
            return false;
        }
    }
    
    return sendMessage(pump_id, "RUN");
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


bool NE500DeviceChannel::setRate(const SendFunction &sendMessage, double currentRate) {
    if (sendMessage(pump_id, "RAT " + formatFloat(currentRate) + " " + rateUnit)) {
        previousRate = currentRate;
        return true;
    }
    return false;
}


bool NE500DeviceChannel::setVolume(const SendFunction &sendMessage, double currentVolume) {
    const double currentAbsVolume = std::abs(currentVolume);
    if (sendMessage(pump_id, "VOL " + formatFloat(currentAbsVolume))) {
        previousAbsVolume = currentAbsVolume;
        return true;
    }
    return false;
}


bool NE500DeviceChannel::setDirection(const SendFunction &sendMessage, Direction currentDirection) {
    const std::string dir_str(currentDirection == Direction::Infuse ? "INF" : "WDR");
    if (sendMessage(pump_id, "DIR " + dir_str)) {
        previousDirection = currentDirection;
        return true;
    }
    return false;
}


END_NAMESPACE_MW




























