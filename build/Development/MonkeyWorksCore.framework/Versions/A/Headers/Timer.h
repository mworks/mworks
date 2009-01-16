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
namespace mw {

class TimeBase : public mw::Component {

protected:

	MonkeyWorksTime time_us;

public:

	TimeBase();
	
	void setTime(MonkeyWorksTime _time);
	void setNow();
	MonkeyWorksTime getTime();
};


class Timer : public Variable, public enable_shared_from_this<Timer> {

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

	void start(MonkeyWorksTime howlongms);
	void startMS(MonkeyWorksTime howlongms);
	void startUS(MonkeyWorksTime howlongus);
	void setExpired(bool has_it);
	bool hasExpired();
	void forceExpired();
	void cleanUp();
	Data getValue();
	void setValue(Data v){ }
	void setValue(Data v, MonkeyWorksTime t){ }
	void setSilentValue(Data _value){ }
	Variable *clone();
	
	
};


void *expireTheTimer(const shared_ptr<Timer> &the_timer);
}

#endif
