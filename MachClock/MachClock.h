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


// if defined, use low-level mach voodoo wherever possible
#define USE_MACH_MOJO

#include "MonkeyWorksCore/Clock.h"
#include "MonkeyWorksCore/Plugin.h"
#include "mach/mach_time.h"
using namespace mw;
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

		
		//#ifdef USE_MACH_MOJO
		mach_timebase_info_data_t    tTBI;
		//	uint64_t cv;
		//#endif
	
		uint64_t baseTime;

    public:
        StandardClock(long interval_microseconds);
    
		virtual MWTime getCurrentTimeMS();
        virtual MWTime getCurrentTimeUS();
		virtual MWTime getCurrentTimeNS();
        
		virtual void sleepMS(MWTime time);
		virtual void sleepUS(MWTime time);
		virtual void sleepNS(MWTime time);
		
        virtual long getInterval();
        virtual void setInterval(long microseconds);

        virtual MWTime getSystemTimeMS();
		virtual MWTime getSystemTimeUS();
		virtual MWTime getSystemTimeNS();
		
        //virtual MWTime getSystemReferenceTime();
        virtual void startClock();
        virtual void stopClock();
                
        virtual shared_ptr<mw::Component> createObject(
								std::map<std::string, std::string> parameters,
								ComponentRegistry *reg){
			return shared_ptr<mw::Component>(new StandardClock(1000));
		}	
};

class StandardClockPlugin : public Plugin {
    public:
	
		virtual void registerComponents(shared_ptr<ComponentRegistry> registry) {
			registry->registerFactory(std::string("MachClock"),
									  (ComponentFactory *)(new StandardClock(0)));
		}
	
		// TODO: write register method
      /*  virtual ExpandableList<ServiceDescription> *
                                                getServiceDescriptions();*/
};

#endif
    