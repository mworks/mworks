/*
 *  VariableRegistryInterface.h
 *  MWorksCore
 *
 *  Created by David Cox on 11/19/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef VARIABLE_REGISTRY_INTERFACE_H_
#define VARIABLE_REGISTRY_INTERFACE_H_


#include "VariableRegistry.h"
#include "EventStreamInterface.h"
#include "EventConstants.h"
#include <boost/shared_ptr.hpp>

using std::string;

namespace mw{

class VariableRegistryInterface {
 
protected:
    shared_ptr<VariableRegistry> registry;

public:
    
    virtual ~VariableRegistryInterface(){ }
    
    virtual shared_ptr<Variable> getVariable(const string &tag);
    virtual shared_ptr<Variable> getVariable(const int code);
    virtual int lookupCodeForTag(const string &tag);
    virtual std::vector<string> getVariableTagNames();
    
};
    
class RegistryAwareEventStreamInterface : public VariableRegistryInterface, public EventStreamInterface {
    
public:
    
    RegistryAwareEventStreamInterface(MessageDomain dom, bool locking);
    
    virtual void registerCallback(string tagname, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(int code, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
};

}

#endif

