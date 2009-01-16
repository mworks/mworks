/*
 *  IODeviceManager.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 8/25/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "IODeviceManager.h"
using namespace mw;

namespace mw {
	IODeviceManager *GlobalIODeviceManager;
}

IODeviceManager::IODeviceManager(){
	devices = new ExpandableList<IODevice>();
}

IODeviceManager::~IODeviceManager(){
	delete devices;
}

void IODeviceManager::addDevice(shared_ptr<IODevice> _dev){
	devices->addReference(_dev);
}


void IODeviceManager::stopAllDeviceIO(){	
	for(int i=0; i < devices->getNElements(); i++){
		(*devices)[i]->stopDeviceIO();
	}
}

