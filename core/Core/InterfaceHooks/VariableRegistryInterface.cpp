/*
 *  VariableRegistryInterface.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 11/19/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "VariableRegistryInterface.h"


BEGIN_NAMESPACE_MW


shared_ptr<Variable> VariableRegistryInterface::getVariable(const std::string &tag) {
	if(registry != NULL){
        return registry->getVariable(tag);
    } else {
        return shared_ptr<Variable>();
    }
}

shared_ptr<Variable> VariableRegistryInterface::getVariable(const int code) {
	if(registry != NULL){
        return registry->getVariable(code);
    } else {
        return shared_ptr<Variable>();
    }
}

int VariableRegistryInterface::lookupCodeForTag(const std::string &tag) {
	shared_ptr<Variable> var = getVariable(tag);
	if(var) {
		return var->getCodecCode();
	} else {
		return -1;
	}
}

std::vector<std::string> VariableRegistryInterface::getVariableTagNames() {
	if(registry != NULL){
        return registry->getVariableTagNames();
    } else {
        return std::vector<std::string>();
    }
}


RegistryAwareEventStreamInterface::RegistryAwareEventStreamInterface(MessageDomain dom, bool locking) : EventStreamInterface(dom, locking){ }

void RegistryAwareEventStreamInterface::registerCallback(string tagname, EventCallback cb, string callback_key){
    int code = lookupCodeForTag(tagname);   
    EventStreamInterface::registerCallback(code, cb, callback_key);
}

void RegistryAwareEventStreamInterface::registerCallback(EventCallback cb, string callback_key){
    EventStreamInterface::registerCallback(cb, callback_key);
}

void RegistryAwareEventStreamInterface::registerCallback(int code, EventCallback cb, string callback_key){
    EventStreamInterface::registerCallback(code, cb, callback_key);
}


END_NAMESPACE_MW
