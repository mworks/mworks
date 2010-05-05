/*
 *  DummyIODevice.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/3/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

/*
 *  NE500.h
 *  MWorksCore
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
		
		
		
		virtual bool initialize(){  return true;  }

		virtual void addChild(std::map<std::string, std::string> parameters,
								ComponentRegistry *reg, 
								shared_ptr<mw::Component> _child){ }
		
		virtual bool startDeviceIO(){  return true; }
		virtual bool stopDeviceIO(){  return true; }


};


class DummyIODeviceFactory : public ComponentFactory {

	shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
													 ComponentRegistry *reg){
													 
			return shared_ptr<mw::Component>(new DummyIODevice());
	}
};


}

#endif






