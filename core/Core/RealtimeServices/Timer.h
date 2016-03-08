#ifndef _TIMER_H
#define _TIMER_H

/*
 *  untitled.h
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on 10/12/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */


#include "Scheduler.h"
#include "Lockable.h"
#include "GenericVariable.h"
#include "Component.h"
#include <boost/thread/mutex.hpp>


BEGIN_NAMESPACE_MW


class TimeBase : public mw::Component {

protected:

	MWTime time_us;

public:

	TimeBase();
	
	void setTime(MWTime _time);
	void setNow();
	MWTime getTime();
};


class Timer : public ReadOnlyVariable {

protected:
	shared_ptr<bool> has_expired;
	shared_ptr<ScheduleTask> schedule_node;
    // DDC: This lock is always taken at the same time as internalLock
    //      It is therefore useless
	//shared_ptr<boost::mutex> schedule_node_lock;
	shared_ptr<boost::mutex> internalLock;

public:
	Timer(VariableProperties *props = 0);
	~Timer();

	void start(MWTime howlongms);
	void startMS(MWTime howlongms);
	void startUS(MWTime howlongus);
	void setExpired(bool has_it);
	bool hasExpired();
	void forceExpired();
	void cleanUp();
    
    Datum getValue() override;
	
};


void *expireTheTimer(const shared_ptr<Timer> &the_timer);


END_NAMESPACE_MW


#endif
