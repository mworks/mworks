#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

/*
 *  ComponentRegistry.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 *
 *  The "Component Registry" is basically just a list of service registries, organized
 *  by type.  So, there might be a Clock service registry, a state system service registry
 *  etc. etc. contained under the component registry, each of which describes and gives
 *  access to the registered components of each type (see ServiceRegistry.h)
 *
 *  Typically, one would go to the component registry to gain access to plugin-defined
 *  components, and the ill-named "service" registries just work under the hood.  The
 *  goal here is to give systematic, programmatic access to everything that gets added to 
 *  the system via plugins, all in one place.
 *
 *  The most important method gets the "Factory" object.  A "factory" object is just a
 *  a simple object that will furnish instances of the object in question, cast as their
 *  core-defined base class.  So, "myHighlySpecializedClock" plugin will provide an 
 *  ClockFactory object that produces instances of "myHighlySpecializedClock", cast as
 *  Clock's. The "factory object" formalism allows the core to instantiate user-defined
 *  classes that inherit from the core's base classes, without having to know anything 
 *  about the inherited class.
 */

#include "ServiceRegistry.h"
#include "Utilities.h"
// A generic component registry



class ComponentRegistry {


    protected:
    
        ExpandableList<ServiceRegistry> registries;
        ExpandableList<ServiceType> types;
        
    public:
    
        ComponentRegistry();
        
        void registerComponent(std::string _tagname, ObjectFactory *_factory, 
															ServiceType _type);
        
        void registerComponent(shared_ptr<ServiceDescription> desc);

        void registerUnresolved(std::string _tagname, ServiceType _type);
            
        ObjectFactory *getFactory(std::string _tagname, ServiceType _type);
        
        bool hasComponent(std::string _tagname, ServiceType _type);        
        int findType(ServiceType _type);
            
        int getNComponentsOfType(ServiceType _type);
        shared_ptr<ServiceDescription> getComponent(ServiceType _type, 
												     int index);
};


#endif