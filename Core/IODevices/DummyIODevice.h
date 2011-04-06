/*
 *  DummyIODevice.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/3/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_DUMMY_IO_H_
#define _DUMMY_IO_H_

#include "IODevice.h"


BEGIN_NAMESPACE_MW


class DummyIODevice : public IODevice {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit DummyIODevice(const ParameterValueMap &parameters) : IODevice(parameters) { }
    
    virtual void addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistryPtr reg, 
                          shared_ptr<Component> child)
    { }

    virtual bool startDeviceIO();
    virtual bool stopDeviceIO();
    
};


END_NAMESPACE_MW


#endif






