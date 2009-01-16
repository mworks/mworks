/**
 * ZenClock.h
 *
 * History:
 * David Cox on Sun Dec 29 2002 - Created.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef STANDARD_CLOCK_H
#define STANDARD_CLOCK_H

#include "MonkeyWorksCore/Clock.h"
#include "MonkeyWorksCore/Plugin.h"

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

class StandardClock : public Clock, public ClockFactory {
    protected:
//    long current_time, interval;

    public:
        StandardClock(long interval_microseconds);

        virtual long getCurrentTimeMS();
		virtual long getCurrentTimeUS();
        virtual long getCurrentTimeNS();

        virtual long getInterval();
        virtual void setInterval(long microseconds);

        virtual long getSystemTime();

        virtual void startClock();
        virtual void stopClock();
                
        virtual Clock *createClock();	
};

class StandardClockPlugin : public Plugin {
    public:
        virtual ExpandableList<ServiceDescription> *
                                                getServiceDescriptions();
};

#endif
