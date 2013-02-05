/*
 *  IODeviceVariableNotification.h
 *  MWorksCore
 *
 *  Created by bkennedy on 12/18/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef IODEVICE_VARIABLE_NOTIFICATION_H
#define IODEVICE_VARIABLE_NOTIFICATION_H

#include "boost/shared_ptr.hpp"
#include "VariableNotification.h"
#include "IODevice.h"


BEGIN_NAMESPACE_MW


	class IODeviceVariableNotification : public VariableNotification{
		
	protected:
		
		weak_ptr<IODevice> io_device;
		
	public:
		
		IODeviceVariableNotification(const shared_ptr<IODevice> &_io_device);
		~IODeviceVariableNotification();		
		virtual void notify(const Datum& data, MWTime timeUS);
		
	};


END_NAMESPACE_MW


#endif
