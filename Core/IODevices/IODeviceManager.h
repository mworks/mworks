#ifndef IODEVICE_MANAGER_H_
#define IODEVICE_MANAGER_H_
/*
 *  IODeviceManager.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 8/25/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "ExpandableList.h"
#include "IODevice.h"
namespace mw {
class IODeviceManager {

protected:

	ExpandableList<IODevice> *devices;
	
public:

	IODeviceManager();
	~IODeviceManager();
	
	void addDevice(shared_ptr<IODevice> _dev);
	
	void stopAllDeviceIO();

};


extern IODeviceManager *GlobalIODeviceManager;
}
#endif

