//
//  USBHIDInputChannel.cpp
//  USBHID
//
//  Created by Christopher Stawarz on 4/4/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "USBHIDInputChannel.h"


BEGIN_NAMESPACE_MW


const std::string USBHIDInputChannel::USAGE_PAGE("usage_page");
const std::string USBHIDInputChannel::USAGE("usage");
const std::string USBHIDInputChannel::VALUE("value");


void USBHIDInputChannel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    
    info.setSignature("iochannel/usbhid_generic_input_channel");
    
    info.addParameter(USAGE_PAGE);
    info.addParameter(USAGE);
    info.addParameter(VALUE);
}


USBHIDInputChannel::USBHIDInputChannel(const ParameterValueMap &parameters) :
    Component(parameters),
    usagePage(parameters[USAGE_PAGE]),
    usage(parameters[USAGE]),
    value(parameters[VALUE])
{
    if (usagePage <= kHIDPage_Undefined) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid HID usage page");
    }
    if (usage <= kHIDUsage_Undefined) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid HID usage");
    }
}


END_NAMESPACE_MW
