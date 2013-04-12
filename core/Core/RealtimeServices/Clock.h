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


class Clock : public Component {

public:
    static const MWTime nanosPerMilli = 1000000LL;
    static const MWTime nanosPerMicro = 1000LL;
    
	virtual ~Clock() { }
	
	MWTime getCurrentTimeMS() { return getCurrentTimeNS() / nanosPerMilli; }
	MWTime getCurrentTimeUS() { return getCurrentTimeNS() / nanosPerMicro; }
	virtual MWTime getCurrentTimeNS();
	
	void sleepMS(MWTime time) { sleepNS(time * nanosPerMilli); }
	void sleepUS(MWTime time) { sleepNS(time * nanosPerMicro); }
	virtual void sleepNS(MWTime time);
	
	virtual void startClock() { }
	virtual void stopClock() { }
	
	MWTime getSystemTimeMS() { return getSystemTimeNS() / nanosPerMilli; }
	MWTime getSystemTimeUS() { return getSystemTimeNS() / nanosPerMicro; }
	virtual MWTime getSystemTimeNS();
    
	MWTime getSystemReferenceTime() { return getSystemBaseTimeNS() / nanosPerMicro; }
	virtual MWTime getSystemBaseTimeNS();
  
    REGISTERED_SINGLETON_CODE_INJECTION(Clock)
  
};


END_NAMESPACE_MW


#endif // !defined(CLOCK_H)

























