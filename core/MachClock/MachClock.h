/**
 * MachClock.h
 *
 * History:
 * David Cox on Sun Dec 29 2002 - Created.
 * Paul Jankunas on 06/02/05 - Fixed spacing.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef STANDARD_CLOCK_H
#define STANDARD_CLOCK_H

#include "MWorksCore/Clock.h"
#include "MWorksCore/Plugin.h"
#include "mach/mach_time.h"


BEGIN_NAMESPACE_MW


//////////////////////////////////////////////////////////////////////////
// Plugin entry point
//
// The extern "C" bit ensures that the name of this function is not mangled 
// (C++ requires name-mangling in order to achieve function overriding).  
// Our plugin host needs to know what name to look for, and for this 
// plugin architecture, that name is 'getPlugin'
//////////////////////////////////////////////////////////////////////////

extern "C"{
    Plugin *getPlugin();
}


class StandardClock : public Clock, public ComponentFactory {
    
    protected:
		mach_timebase_info_data_t    tTBI;
		uint64_t baseTime;

    public:
        StandardClock();
    
		MWTime getCurrentTimeNS() MW_OVERRIDE;
        
		void sleepNS(MWTime time) MW_OVERRIDE;

		MWTime getSystemTimeNS() MW_OVERRIDE;
        MWTime getSystemBaseTimeNS() MW_OVERRIDE;
                
        shared_ptr<mw::Component>
        createObject(std::map<std::string, std::string> parameters, ComponentRegistry *reg) MW_OVERRIDE {
			return shared_ptr<mw::Component>(new StandardClock);
		}
	
};


class StandardClockPlugin : public Plugin {
    public:
		virtual void registerComponents(shared_ptr<ComponentRegistry> registry) {
			registry->registerFactory("MachClock", new StandardClock);
		}
};


END_NAMESPACE_MW


#endif
    