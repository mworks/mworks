/*
 *  IODeviceVariableNotification.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 12/18/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "IODeviceVariableNotification.h"
#include "StandardVariables.h"
// ===================================


BEGIN_NAMESPACE_MW


IODeviceVariableNotification::IODeviceVariableNotification
        (const shared_ptr<IODevice> &_io_device) : VariableNotification() {
	io_device = weak_ptr<IODevice>(_io_device);
}

IODeviceVariableNotification::~IODeviceVariableNotification() {}

void IODeviceVariableNotification::notify(const Datum& data, MWTime timeUS) {
	if(data.getInteger() == IDLE) {
		shared_ptr<IODevice> io_dev_shared = io_device.lock();
        if(io_dev_shared != NULL){ 
            io_dev_shared->stopDeviceIO();
        }
	}
}


END_NAMESPACE_MW
