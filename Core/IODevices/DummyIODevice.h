/*
 *  DummyIODevice.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/3/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

/*
 *  NE500.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_DUMMY_IO_H_
#define _DUMMY_IO_H_

#include "IODevice.h"

namespace mw {
using namespace std;


class DummyIODevice : public IODevice {


	public:
	
		DummyIODevice(){ }
		
		
		
		// specify what this device can do
		virtual ExpandableList<IOCapability> *getCapabilities(){ return NULL; }
		virtual bool mapRequestsToChannels(){  return true;  }
		virtual bool initializeChannels(){  return true;  }
		virtual bool startup(){  return true;  }

		virtual void addChild(std::map<std::string, std::string> parameters,
								mwComponentRegistry *reg, 
								shared_ptr<mw::Component> _child){ }
		
		virtual bool startDeviceIO(){  return true; }
		virtual bool stopDeviceIO(){  return true; }
		
        virtual bool attachPhysicalDevice(){ return true; }
        
		
		// this will stop anyIO behavior on a device and put the device in a shutdown state (if the device has one) -- e.g. turn off x-ray
		virtual bool shutdown(){ return true; }


};


class DummyIODeviceFactory : public ComponentFactory {

	shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
													 mwComponentRegistry *reg){
													 
			return shared_ptr<mw::Component>(new DummyIODevice());
	}
};


}

#endif






