#ifndef CLOCK_H
#define CLOCK_H

/*
 *  Clock.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 * Paul Jankunas on 07/25/05 - Added getSystemReferenceTime
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *  Copyright (c) 2002 MIT. All rights reserved.
 *
 */

#include "MonkeyWorksTypes.h"
#include "Component.h"
#include "RegisteredSingleton.h"
namespace mw {
// TODO: config.h 

class Clock : public mw::Component, public RegisteredSingleton<Clock> {
	
protected:
	long current_time, interval;

public:
    
	Clock(long interval_microseconds);
	virtual ~Clock();
	
	virtual MonkeyWorksTime getCurrentTime();	
	virtual MonkeyWorksTime getCurrentTimeMS();
	virtual MonkeyWorksTime getCurrentTimeUS();
	virtual MonkeyWorksTime getCurrentTimeNS();
	
	virtual void sleepMS(MonkeyWorksTime time);
	virtual void sleepUS(MonkeyWorksTime time);
	virtual void sleepNS(MonkeyWorksTime time);
	
	virtual long getInterval();
	virtual void setInterval(long micro);
	
	virtual void startClock();
	virtual void stopClock();
	
	virtual MonkeyWorksTime getSystemTimeMS();
	virtual MonkeyWorksTime getSystemTimeUS();
	virtual MonkeyWorksTime getSystemTimeNS();
	/**
	 * Returns the difference between the system start time
	 * and the UNIX reference time Jan 1 1970. 
	 */
	virtual MonkeyWorksTime getSystemReferenceTime();
};

}
#endif

