/*
 *  DummyIODevice.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/3/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "DummyIODevice.h"


BEGIN_NAMESPACE_MW


void DummyIODevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    info.setSignature("iodevice/dummy");
}


bool DummyIODevice::startDeviceIO() {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Started I/O on dummy device");
    return true;
}


bool DummyIODevice::stopDeviceIO() {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Stopped I/O on dummy device");
    return true;
}


END_NAMESPACE_MW
