/*
 *  IODeviceVariableNotification.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 12/18/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "IODeviceVariableNotification.h"
// ===================================

using namespace mw;
IODeviceVariableNotification::IODeviceVariableNotification
(const shared_ptr<IODevice> &_io_device) : VariableNotification() {
	io_device = _io_device;
}

IODeviceVariableNotification::~IODeviceVariableNotification() {}

void IODeviceVariableNotification::notify(const Data& data, MonkeyWorksTime timeUS) {
	if(data.getInteger() == STOPPING || data.getInteger() == IDLE) {
		io_device->stopDeviceIO();
	}
}

