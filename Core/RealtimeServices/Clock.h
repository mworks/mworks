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

#include "MWorksTypes.h"
#include "Component.h"
#include "RegisteredSingleton.h"
namespace mw {
// TODO: config.h 

class Clock : public mw::Component {//, public RegisteredSingleton<Clock> {
	
protected:
	long current_time, interval;

public:
    
	Clock(long interval_microseconds);
	virtual ~Clock();
	
	virtual MWTime getCurrentTime();	
	virtual MWTime getCurrentTimeMS();
	virtual MWTime getCurrentTimeUS();
	virtual MWTime getCurrentTimeNS();
	
	virtual void sleepMS(MWTime time);
	virtual void sleepUS(MWTime time);
	virtual void sleepNS(MWTime time);
	
	virtual long getInterval();
	virtual void setInterval(long micro);
	
	virtual void startClock();
	virtual void stopClock();
	
	virtual MWTime getSystemTimeMS();
	virtual MWTime getSystemTimeUS();
	virtual MWTime getSystemTimeNS();
	/**
	 * Returns the difference between the system start time
	 * and the UNIX reference time Jan 1 1970. 
	 */
	virtual MWTime getSystemReferenceTime();
  
  REGISTERED_SINGLETON_CODE_INJECTION(Clock)
  
};

}
#endif

