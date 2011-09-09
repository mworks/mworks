/*
 *  ServiceRegistry.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "ServiceRegistry.h"
#include <string.h>





//class ServiceRegistry::

ServiceRegistry::ServiceRegistry(){ }

void ServiceRegistry::registerService(std::string tag, 
									  ObjectFactory *factory, 
									  ServiceType type){
	shared_ptr<ServiceDescription> p(new ServiceDescription(tag, factory, type));
    services.addElement(p);
};

void ServiceRegistry::registerService(shared_ptr<ServiceDescription> desc){
    services.addElement(desc);
}


void ServiceRegistry::registerUnresolvedService(std::string tag){
    registerUnresolved(tag, M_UNRESOLVED_SERVICE);
}

void ServiceRegistry::registerUnresolved(std::string tag, ServiceType type){
    if(!hasUnresolvedService(tag)){
		shared_ptr<ServiceDescription> d(new ServiceDescription(tag, NULL, type));
        unresolved.addElement(d);
    }
}

ObjectFactory *ServiceRegistry::getFactory(std::string matchtag){

    for(int i = 0; i < services.getNElements(); i++){
        shared_ptr<ServiceDescription> description = services[i];
        if(matchtag == description->getTagName()){
            return description->getFactory();
        }
    }
    
    // throw an exception?
    return NULL;
}


bool ServiceRegistry::hasService(std::string matchtag){
    return hasService(matchtag, &services);
}

bool ServiceRegistry::hasUnresolvedService(std::string matchtag){
    return hasService(matchtag, &unresolved);
}

bool ServiceRegistry::hasService(std::string matchtag, 
								  ExpandableList<ServiceDescription> *list){
    for(int i = 0; i < list->getNElements(); i++){
        shared_ptr<ServiceDescription> description = (*list)[i];
        if(matchtag == description->getTagName()){
            return true;
        }
    }
    
    return false;
}

int ServiceRegistry::getNServices(){
    return services.getNElements();
}

shared_ptr<ServiceDescription> ServiceRegistry::getService(int i){
    //todo: error check array bounds
    return services[i];
}
