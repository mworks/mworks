/*
 *  ComponentRegistry.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */
 
#include "Utilities.h"
#include "ComponentRegistry.h"


ComponentRegistry::ComponentRegistry(){
    
    for(int i = 0; i < M_N_SERVICE_TYPES; i++){
        types.addElement((ServiceType)i);
        shared_ptr<ServiceRegistry> newregistry(new ServiceRegistry());
        registries.addElement(newregistry);
    }
}


void ComponentRegistry::registerComponent(std::string _tagname, 
										   ObjectFactory *_factory, 
										   ServiceType _type){

    int index = findType(_type);

    //fprintf(stderr, "%s:%d\n", _tagname, index);fflush(stderr);
    if(index >= 0){
        
        shared_ptr<ServiceRegistry> theregistry = registries[index];
        theregistry->registerService(_tagname, _factory, _type);
        return;
    } else {
    
        types.addElement(_type);
        shared_ptr<ServiceRegistry> newregistry(new ServiceRegistry());
        registries.addElement(newregistry);
        newregistry->registerService(_tagname, _factory, _type);
    }
}

void ComponentRegistry::registerComponent(shared_ptr<ServiceDescription> desc){
    ServiceType _type = desc->getType();
    int index = findType(_type);
    
    //fprintf(stderr, "%s:%d\n", desc->getTagName(), index);fflush(stderr);

    
    if(index >= 0){
        
        shared_ptr<ServiceRegistry> theregistry = registries[index];
        theregistry->registerService(desc);
        return;
    } else {
    
        types.addElement(_type);
        shared_ptr<ServiceRegistry> newregistry(new ServiceRegistry());
        registries.addElement(newregistry);
        newregistry->registerService(desc);
    }
}

void ComponentRegistry::registerUnresolved(std::string _tagname, 
											ServiceType _type){
    int index = findType(_type);
    
    if(index >= 0){
        (registries[index])->registerUnresolved(_tagname, _type);
        return;
    } else {
        types.addElement(_type);
        shared_ptr<ServiceRegistry> newregistry(new ServiceRegistry());
        registries.addElement(newregistry);
        newregistry->registerUnresolved(_tagname, _type);
    }
}
    
ObjectFactory *ComponentRegistry::getFactory(std::string _tagname, 
											   ServiceType _type){

    // find the correct registry
    int index = findType(_type);
    
   // fprintf(stderr, "fetching %s:%d\n", _tagname, _type);fflush(stderr);
    
    if(index >= 0){
        ObjectFactory *factory = (registries[index])->getFactory(_tagname);
        if(factory == NULL){
            fprintf(stderr, "Service not found (%s, %d)\n", _tagname.c_str(), _type);fflush(stderr);
            return NULL;
        }
            
        return factory;
    }
    
    // throw error?
    return NULL;
}


bool ComponentRegistry::hasComponent(std::string _tagname, ServiceType _type){
    int index = findType(_type);
    if(index >= 0){
        return (registries[index])->hasService(_tagname);
    }
    
    return false;
}


int ComponentRegistry::findType(ServiceType _type){
    for(int i = 0; i < types.getNElements(); i++){
        if(_type == *(types[i])){
            return i;
        }
    }
    
    return -1;
}

int ComponentRegistry::getNComponentsOfType(ServiceType _type){
    int index = findType(_type);
    if(index < 0){
        return 0;
    }
    
    return (registries[index])->getNServices();
}

shared_ptr<ServiceDescription> ComponentRegistry::getComponent(
														ServiceType _type, 
														int index){
    int typeindex = findType(_type);
    return (registries[typeindex])->getService(index);
}



