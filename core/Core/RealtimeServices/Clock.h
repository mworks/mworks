/*
 *  Clock.h
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Paul Jankunas on 07/25/05 - Added getSystemReferenceTime
 *  Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *  Copyright (c) 2002 MIT. All rights reserved.
 *
 */

#ifndef CLOCK_H
#define CLOCK_H

#include "MWorksTypes.h"
#include "Component.h"
#include "RegisteredSingleton.h"


BEGIN_NAMESPACE_MW


class Clock {

public:
    static constexpr MWTime nanosPerMilli = 1000000LL;
    static constexpr MWTime nanosPerMicro = 1000LL;
    
	virtual ~Clock() { }
	
	MWTime getCurrentTimeMS() { return getCurrentTimeNS() / nanosPerMilli; }
	MWTime getCurrentTimeUS() { return getCurrentTimeNS() / nanosPerMicro; }
    virtual MWTime getCurrentTimeNS() { return 0; }
	
	void sleepMS(MWTime time) { sleepNS(time * nanosPerMilli); }
	void sleepUS(MWTime time) { sleepNS(time * nanosPerMicro); }
    virtual void sleepNS(MWTime time) { }
    
    virtual void yield() { }
	
	virtual void startClock() { }
	virtual void stopClock() { }
	
	MWTime getSystemTimeMS() { return getSystemTimeNS() / nanosPerMilli; }
	MWTime getSystemTimeUS() { return getSystemTimeNS() / nanosPerMicro; }
    virtual MWTime getSystemTimeNS() { return 0; }
    
	MWTime getSystemReferenceTime() { return getSystemBaseTimeNS() / nanosPerMicro; }
    virtual MWTime getSystemBaseTimeNS() { return 0; }
  
    REGISTERED_SINGLETON_CODE_INJECTION(Clock)
  
};


END_NAMESPACE_MW


#endif // !defined(CLOCK_H)
