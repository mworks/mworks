#ifndef PLUGIN_H
#define PLUGIN_H

/*
 *  Plugin.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 *  In addition to it's actual payload (new objects derived from base objects in the
 *  core), the plugin bundle needs to define a simple Plugin object that serves up
 *  descriptions of what these new objects are, and how to get at them. The Plugin object 
 *  basically just advertises what kinds of services are available and provides pointers
 *  to "Factory" objects that spit out instances of the new, user-defined objects.
 *
 *  When a plugin is read, the getServiceDescription() method is called, and all of the 
 *  ServiceDescriptions are registered into a standard mwComponentRegistry.  The core can then
 *  turn to this registry as needed, and use the user-defined ObjectFactories to instantiate the
 *  instances of the user-defined objects.
 *
 *  In practice, this is less complicated than it sounds, and it should be fairly clear from the
 *  example plugins what needs to be done.
 */

#include "ComponentRegistry_new.h"
#include "ComponentFactory.h"
namespace mw {
	
	class Plugin {
		
    public:    
        virtual ~Plugin() { }
		virtual void registerComponents(shared_ptr<mwComponentRegistry> registry) { }
		
	};
}

#endif
