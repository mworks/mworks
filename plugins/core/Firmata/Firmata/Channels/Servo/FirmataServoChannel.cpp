//
//  FirmataServoChannel.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 11/10/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "FirmataServoChannel.hpp"


BEGIN_NAMESPACE_MW


const std::string FirmataServoChannel::MIN_PULSE_WIDTH("min_pulse_width");
const std::string FirmataServoChannel::MAX_PULSE_WIDTH("max_pulse_width");


void FirmataServoChannel::describeComponent(ComponentInfo &info) {
    FirmataChannel::describeComponent(info);
    info.setSignature("iochannel/firmata_servo");
    info.addParameter(MIN_PULSE_WIDTH, "544");
    info.addParameter(MAX_PULSE_WIDTH, "2400");
}


FirmataServoChannel::FirmataServoChannel(const ParameterValueMap &parameters) :
    FirmataChannel(parameters),
    minPulseWidth(parameters[MIN_PULSE_WIDTH]),
    maxPulseWidth(parameters[MAX_PULSE_WIDTH])
{
    if (minPulseWidth < minAllowedPulseWidth ||
        minPulseWidth > maxAllowedPulseWidth ||
        maxPulseWidth < minAllowedPulseWidth ||
        maxPulseWidth > maxAllowedPulseWidth)
    {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              (boost::format("min/max pulse widths must be between %d and %d (inclusive)")
                               % int(minAllowedPulseWidth)
                               % int(maxAllowedPulseWidth)));
    }
    if (maxPulseWidth <= minPulseWidth) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s must be greater than %s") % MAX_PULSE_WIDTH % MIN_PULSE_WIDTH);
    }
}


END_NAMESPACE_MW




















