#ifndef STANDARD_CLOCK_H
#define STANDARD_CLOCK_H

/*
 *  StandardClock.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sun Dec 29 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "MonkeyWorksCore/Clock.h"
#include "MonkeyWorksCore/Plugin.h"


///////////////////////////////////////////////////////////////////////////////////////
// Plugin entry point
//
// The extern "C" bit ensures that the name of this function is not mangled (C++
// requires name-mangling in order to achieve function overriding.  Our plugin host
// needs to know what name to look for, and for this plugin architecture, that name
// is 'getPlugin'
///////////////////////////////////////////////////////////////////////////////////////

extern "C"{
    Plugin *getPlugin();
}




class StandardClock : public Clock, public ClockFactory {

//    long current_time, interval;

    public:
    
        StandardClock(long interval_microseconds);
    
        virtual long getCurrentTimeMS();
        virtual long getCurrentTimeUS();
		virtual long getCurrentTimeNS();
        
		virtual long getSystemTime();
        
        virtual long getInterval();
        virtual void setInterval(long microseconds);

        virtual void startClock();
        virtual void stopClock();
        
        
        virtual Clock *createClock();
	
};


class StandardClockPlugin : public Plugin {
    
    
    virtual ExpandableList<ServiceDescription> *getServiceDescriptions(){
    
        ExpandableList<ServiceDescription> *list = new ExpandableList<ServiceDescription>();
        
        // only one service in this plugin (just the scheduler), but a plugin could, in principle,
        // contain a whole suite of related services.
        
        list->addElement(new ServiceDescription("StandardClock", (ObjectFactory *)(new StandardClock(1000)), M_CLOCK_COMPONENT));
    
        return list;
    }
};

#endif
    